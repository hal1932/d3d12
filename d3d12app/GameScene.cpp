#include "GameScene.h"
#include "Graphics.h"
#include <cmath>


GameScene::~GameScene() {}


void GameScene::Setup(Graphics& g)
{
	taskQueue_.Setup(cThreadCount);

	auto pDevice = g.DevicePtr();
	auto pNativeDevice = pDevice->NativePtr();

	auto& commandListPtrs = commandLists_.CreateCommandLists("main", 0);
	commandLists_.CreateCommandLists("model_bundles", -1);
	commandLists_.CommitExecutionOrders();

	auto pCommandList = g.CreateCommandList(CommandList::SubmitType::Direct, 1);
	commandListPtrs.push_back(pCommandList);

	auto meshCount = 0;
	for (auto& pModel : modelPtrs_)
	{
		pModel = std::make_unique<Model>();
		pModel->Setup(pDevice, "assets/test_anim.fbx");
		pModel->UpdateSubresources(pCommandList, g.CommandQueuePtr());
		meshCount += pModel->MeshCount();
	}

	cbSrUavHeap_.CreateHeap(pDevice, { HeapDesc::ViewType::CbSrUaView, static_cast<int>(modelPtrs_.size()) * 2 + meshCount });

	for (auto& pModel : modelPtrs_)
	{
		pModel->SetupBuffers(&cbSrUavHeap_);
	}

	//for (auto i = 0; i < cModelGridSize; ++i)
	//{
	//	for (auto j = 0; j < cModelGridSize; ++j)
	//	{
	//		for (auto k = 0; k < cModelGridSize; ++k)
	//		{
	//			const auto index = i * cModelGridSize * cModelGridSize + j * cModelGridSize + k;

	//			auto pModel = modelPtrs_[index];

	//			auto t = pModel->TransformPtr();
	//			t->SetScaling(0.1f, 0.1f, 0.1f);
	//			t->SetTranslation(-2.0f + i * 0.3f, -2.0f + j * 0.3f, -1.0f + k * 0.3f);
	//			t->UpdateMatrix();
	//		}
	//	}
	//}

	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER1 params[3];
		params[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		params[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
		params[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
		desc.Init_1_1(
			_countof(params), params,
			1, &sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
		);

		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;

		ThrowIfFailed(
			D3DX12SerializeVersionedRootSignature(
				&desc,
				D3D_ROOT_SIGNATURE_VERSION_1,
				&pSignature,
				&pError));

		ThrowIfFailed(
			pNativeDevice->CreateRootSignature(
				0,
				pSignature->GetBufferPointer(),
				pSignature->GetBufferSize(),
				IID_PPV_ARGS(&pRootSignature_)));
	}

	{
		for (auto& pModel : modelPtrs_)
		{
			shaders_.LoadFromModelMaterial(&pModel->FbxModel());
		}

		std::sort(
			modelPtrs_.begin(), modelPtrs_.end(),
			[](const auto& lhs, const auto& rhs) { return lhs->ShaderHash() < rhs->ShaderHash();}
		);

		CD3DX12_RASTERIZER_DESC descRS(D3D12_DEFAULT);
		CD3DX12_BLEND_DESC descBS(D3D12_DEFAULT);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = pRootSignature_.Get();
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

		for (const auto& name : { "Simple2" })
		{
			desc.InputLayout = shaders_.InputLayout(name);
			desc.VS = shaders_.VertexShader(name);
			desc.PS = shaders_.PixelShader(name);

			ComPtr<ID3D12PipelineState> pPso;
			ThrowIfFailed(pNativeDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pPso)));
			pPipelineStates_[name] = pPso;
		}
	}

	CreateModelCommand(g);
}


void GameScene::CreateModelCommand(Graphics& g)
{
	auto& lists = commandLists_.GetCommandList("model_bundles");
	auto& models = modelPtrs_;

	const auto threadCount = taskQueue_.ThreadCount();
	const auto countPerThread = models.size() / threadCount;

	for (auto i = 0; i < threadCount; ++i)
	{
		auto count = countPerThread;
		if (i == threadCount - 1)
		{
			count = models.size() - countPerThread * i;
		}

		const auto start = countPerThread * i;
		const auto end = start + count;

		auto pList = g.CreateCommandList(CommandList::SubmitType::Bundle, 1);
		lists.push_back(pList);

		pList->Open(nullptr);

		auto pNativeList = pList->GraphicsList();

		auto pHeap = cbSrUavHeap_.NativePtr();
		pNativeList->SetDescriptorHeaps(1, &pHeap);
		pNativeList->SetGraphicsRootSignature(pRootSignature_.Get());
		pNativeList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ulonglong lastShader = 0ULL;
		for (auto i = start; i < end; ++i)
		{
			auto& pModel = models[i];
			const auto shader = pModel->ShaderHash();
			if (lastShader != shader)
			{
				const auto& name = shaders_.Name(shader);
				pNativeList->SetPipelineState(pPipelineStates_[name].Get());
				lastShader = shader;
			}
			pModel->CreateDrawCommand(pNativeList);
		}

		pList->Close();
	}
}


void GameScene::Calc()
{
	cpuTimer_.Start(100, "calc");

	const auto threadCount = taskQueue_.ThreadCount();

	auto& models = modelPtrs_;
	const auto countPerThread = models.size() / threadCount;

	for (auto i = 0; i < threadCount; ++i)
	{
		auto count = countPerThread;
		if (i == threadCount - 1)
		{
			count = models.size() - countPerThread * i;
		}

		const auto start = countPerThread * i;
		const auto end = start + count;

		const auto frameIndex = frameIndex_;

		taskQueue_.Enqueue([i, start, end, frameIndex, &models]()
		{
			for (auto j = start; j < end; ++j)
			{
				auto t = models[j]->TransformPtr();
				t->SetTranslation(0.0f, std::sinf(static_cast<float>(frameIndex * 0.1)), 0.0f);
				t->UpdateMatrix();
			}
		});
	}

	++frameIndex_;

	cpuTimer_.Stop(100);
}


void GameScene::Draw(Graphics& g, GpuStopwatch* pStopwatch)
{
	cpuTimer_.Start(200, "all");

	cpuTimer_.Start(201, "camera");
	{
		auto& c = camera_;

		c.SetPosition({ 10.0f, 5.0f, -10.0f });
		c.SetFocus({ 0.0f, 0.0f, 0.0f });
		c.SetUp({ 0.0f, 1.0f, 0.0f });

		c.SetFovY(DirectX::XM_PIDIV4);
		c.SetAspect(g.ScreenPtr()->AspectRatio());
		c.SetNearPlane(0.1f);
		c.SetFarPlane(1000.0f);

		c.UpdateMatrix();

		for (auto i = 0; i < taskQueue_.ThreadCount(); ++i)
		{
			cameraBuffers_[i].View = c.View();
			cameraBuffers_[i].Proj = c.Proj();
		}
	}
	cpuTimer_.Stop(201);

	auto pGraphicsList = commandLists_.GetCommandList("main")[0];
	pGraphicsList->Open(nullptr);

	auto pNativeGraphicsList = pGraphicsList->GraphicsList();

	{
		auto heap = cbSrUavHeap_.NativePtr();
		pNativeGraphicsList->SetDescriptorHeaps(1, &heap);
	}
	pStopwatch->Start(g.CommandQueuePtr()->NativePtr(), pNativeGraphicsList);

	cpuTimer_.Start(210, "RS");
	{
		const auto& screen = g.ScreenPtr()->Desc();
		viewport_ = { 0.0f, 0.0f, (float)screen.Width, (float)screen.Height, 0.0f, 1.0f };
		pNativeGraphicsList->RSSetViewports(1, &viewport_);

		scissorRect_ = { 0, 0, screen.Width, screen.Height };
		pNativeGraphicsList->RSSetScissorRects(1, &scissorRect_);
	}
	cpuTimer_.Stop(210);

	cpuTimer_.Start(220, "OM");
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			g.CurrentRenderTargetPtr()->NativePtr(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		pNativeGraphicsList->ResourceBarrier(1, &barrier);

		auto handleRTV = g.CurrentRenderTargetPtr()->CpuDescriptorHandle();
		auto handleDSV = g.DepthStencilPtr()->CpuDescriptorHandle();

		pNativeGraphicsList->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);

		FLOAT clearValue[] = { 0.2f, 0.2f, 0.5f, 1.0f };
		pNativeGraphicsList->ClearRenderTargetView(handleRTV, clearValue, 0, nullptr);
		pNativeGraphicsList->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
	cpuTimer_.Stop(220);

	cpuTimer_.Start(225, "wait_calc");
	{
		taskQueue_.WaitAll();
	}
	cpuTimer_.Stop(225);

	const auto threadCount = taskQueue_.ThreadCount();

	cpuTimer_.Start(230, "models_cbuffer_update");
	{
		const auto countPerThread = modelPtrs_.size() / threadCount;

		for (auto i = 0; i < threadCount; ++i)
		{
			auto count = countPerThread;
			if (i == threadCount - 1)
			{
				count = modelPtrs_.size() - countPerThread * i;
			}

			const auto start = countPerThread * i;
			const auto end = start + count;

			taskQueue_.Enqueue([i, start, end, this]()
			{
				for (auto j = start; j < end; ++j)
				{
					auto& pModel = modelPtrs_[j];
					modelTransformBuffers_[i].World = pModel->TransformPtr()->Matrix();
					pModel->SetTransform(modelTransformBuffers_[i].World, cameraBuffers_[i]);
				}
			});
		}
	}
	cpuTimer_.Stop(230);

	cpuTimer_.Start(240, "models-draw");
	{
		for (auto pBundle : commandLists_.GetCommandList("model_bundles"))
		{
			//taskQueue.Enqueue([pNativeGraphicsList, pBundle]()
			//{
			pNativeGraphicsList->ExecuteBundle(pBundle->GraphicsList());
			//});
		}
	}
	cpuTimer_.Stop(240);

	cpuTimer_.Start(250, "wait_OM");
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			g.CurrentRenderTargetPtr()->NativePtr(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
		);
		pNativeGraphicsList->ResourceBarrier(1, &barrier);
	}
	cpuTimer_.Stop(250);

	pStopwatch->Stop();

	cpuTimer_.Start(260, "close_cmdlist");
	{
		pGraphicsList->Close();
	}
	cpuTimer_.Stop(260);

	cpuTimer_.Start(265, "wait_models_cbuffer_update");
	{
		taskQueue_.WaitAll();
	}
	cpuTimer_.Stop(265);

	cpuTimer_.Start(270, "submit_cmdlist");
	{
		commandLists_.Execute(g.CommandQueuePtr());
	}
	cpuTimer_.Stop(270);

	cpuTimer_.Start(280, "swap");
	{
		g.SwapBuffers(1);
	}
	cpuTimer_.Stop(280);

	cpuTimer_.Start(290, "wait_cmdlist");
	{
		g.WaitForCommandExecution();
	}
	cpuTimer_.Stop(290);

	cpuTimer_.Stop(200);
	if (cpuTimer_.DumpAll(200, 60))
	{
		cpuTimer_.Reset();
	}
}
