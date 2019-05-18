#include "ModelPass.h"
#include "Model.h"

#include <set>
#include <numeric>


void ModelPass::SetupCommandList(Device* pDevice, CommandQueue* pCommandQueue)
{
	pRootCommandQueue_ = pCommandQueue;

	auto& mainCmds = commandListQueue_.CreateGroup("main", 0);
	commandListQueue_.CreateGroup("bundles", -1);
	commandListQueue_.CommitExecutionOrders();

	pMainCommandList_ = new CommandList();
	pMainCommandList_->Create(pDevice, CommandList::SubmitType::Direct, 1);
	mainCmds.push_back(pMainCommandList_);
}


void ModelPass::SetupGpuResources(Device* pDevice) {}


void ModelPass::SetupRenderPipeline(Device* pDevice)
{
	// シェーダ
	std::set<u64> modelShaderHashes;
	for (auto& model : *pModels_)
	{
		const auto shaderHash = shaders_.LoadFromModelMaterial(&model.FbxModel());
		modelShaderHashes.insert(shaderHash);
	}

	std::sort(
		pModels_->begin(), pModels_->end(),
		[](const auto& lhs, const auto& rhs) { return lhs.ShaderHash() < rhs.ShaderHash();}
	);

	// PSO
	CD3DX12_RASTERIZER_DESC descRS(D3D12_DEFAULT);
	CD3DX12_BLEND_DESC descBS(D3D12_DEFAULT);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.RasterizerState = descRS;
	desc.BlendState = descBS;
	desc.DepthStencilState.DepthEnable = TRUE;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.StencilEnable = FALSE;
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;

	for (const auto shaderHash : modelShaderHashes)
	{
		CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		const auto shaderName = shaders_.Name(shaderHash);
		RootSignatureDesc rootSigDesc(shaders_.VertexShader(shaderName), shaders_.PixelShader(shaderName));
		rootSignature_.Create(pDevice, rootSigDesc, 1, &sampler);

		desc.pRootSignature = rootSignature_.NativePtr();
		desc.InputLayout = shaders_.InputLayout(shaderName);
		desc.VS = shaders_.VertexShaderBytecode(shaderName);
		desc.PS = shaders_.PixelShaderBytecode(shaderName);

		ComPtr<ID3D12PipelineState> pPso;
		ThrowIfFailed(pDevice->NativePtr()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pPso)));
		pPipelineStateObjs_[shaderHash] = pPso;
	}

	// Bundle
	auto& modelCommandLists = commandListQueue_.GetGroup("bundles");

	const auto threadCount = std::min<size_t>(pModels_->Size(), pTaskQueue_->ThreadCount());
	const auto countPerThread = pModels_->Size() / threadCount + 1;

	for (auto i = 0; i < threadCount; ++i)
	{
		auto pList = new CommandList();
		pList->Create(pDevice, CommandList::SubmitType::Bundle, 1);
		modelCommandLists.push_back(pList);
	}

	auto& models = *pModels_;
	for (auto i = 0; i < threadCount; ++i)
	{
		const auto threadIndex = i;
		pTaskQueue_->Enqueue([&, threadIndex]()
		{
			auto count = countPerThread;
			if (threadIndex == threadCount - 1)
			{
				count = models.Size() - countPerThread * threadIndex;
			}

			const auto start = countPerThread * threadIndex;
			const auto end = start + count;

			auto pList = modelCommandLists[threadIndex];

			pList->Open(nullptr);

			auto pNativeList = pList->GraphicsList();

			auto pHeap = pModelResourceViewHeap_->NativePtr();
			pNativeList->SetDescriptorHeaps(1, &pHeap);

			pNativeList->SetGraphicsRootSignature(rootSignature_.NativePtr());
			pNativeList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			u64 lastShader = 0ULL;
			for (auto i = start; i < end; ++i)
			{
				auto& model = models[i];
				const auto shader = model.ShaderHash();
				if (lastShader != shader)
				{
					const auto& name = shaders_.Name(shader);
					pNativeList->SetPipelineState(pPipelineStateObjs_[shader].Get());
					lastShader = shader;
				}
				model.CreateDrawCommand(pNativeList, pCameraCbv_);
			}

			pList->Close();
		});
	}

	pTaskQueue_->WaitAll();
}


void ModelPass::Calc() {}


void ModelPass::OpenDraw()
{
	pMainCommandList_->Open(nullptr);
	auto pNativeGraphicsList = pMainCommandList_->GraphicsList();

	// RS
	{
		pNativeGraphicsList->RSSetViewports(1, &viewport_);
		pNativeGraphicsList->RSSetScissorRects(1, &scissorRect_);
	}

	// OM
	{
		CD3DX12_RESOURCE_BARRIER barriers[8];
		D3D12_CPU_DESCRIPTOR_HANDLE handleRTVs[8];
		for (UINT i = 0; i < renderTargetCount_; ++i)
		{
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				pRenderTargetPtrs_[i]->NativePtr(),
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
			);
			handleRTVs[i] = pRenderTargetPtrs_[i]->CpuDescriptorHandle();
		}
		pNativeGraphicsList->ResourceBarrier(renderTargetCount_, barriers);

		auto handleDSV = pDepthStencil_->CpuDescriptorHandle();

		pNativeGraphicsList->OMSetRenderTargets(renderTargetCount_, handleRTVs, FALSE, &handleDSV);

		FLOAT clearValue[] = { 0.2f, 0.2f, 0.5f, 1.0f };
		for (UINT i = 0; i < renderTargetCount_; ++i)
		{
			pNativeGraphicsList->ClearRenderTargetView(handleRTVs[i], clearValue, 0, nullptr);
		}
		pNativeGraphicsList->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}


void ModelPass::Draw()
{
	auto pNativeGraphicsList = pMainCommandList_->GraphicsList();

	// models
	{
		auto pHeap = pModelResourceViewHeap_->NativePtr();
		pNativeGraphicsList->SetDescriptorHeaps(1, &pHeap);

		for (auto pBundle : commandListQueue_.GetGroup("bundles"))
		{
			pNativeGraphicsList->ExecuteBundle(pBundle->GraphicsList());
		}
	}
}


void ModelPass::CloseDraw(CommandQueue* pCommandQueue)
{
	auto pNativeGraphicsList = pMainCommandList_->GraphicsList();

	// wait OM
	{
		CD3DX12_RESOURCE_BARRIER barriers[8];
		for (UINT i = 0; i < renderTargetCount_; ++i)
		{
			barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				pRenderTargetPtrs_[i]->NativePtr(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
			);
		}
		pNativeGraphicsList->ResourceBarrier(renderTargetCount_, barriers);
	}

	pMainCommandList_->Close();
	if (pCommandQueue == nullptr)
	{
		pCommandQueue = pRootCommandQueue_;
	}
	commandListQueue_.Execute(pCommandQueue);
}

