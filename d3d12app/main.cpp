#include <stdlib.h>
#include <crtdbg.h>

#include <memory>
#include <map>
#include <array>

#include <wrl.h>
#include <Windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <algorithm>

#include <lib.h>
#include "Graphics.h"
#include "Model.h"

const int cScreenWidth = 1280;
const int cScreenHeight = 720;
const int cBufferCount = 2;
const int cModelGridSize = 1;
const int cThreadCount = 3;

struct Scene
{
	ComPtr<ID3D12RootSignature> pRootSignature;
	float rotateAngle;

	std::array<Model*, cModelGridSize * cModelGridSize * cModelGridSize> modelPtrs;
	std::map<tstring, ComPtr<ID3D12PipelineState>> pPipelineStates;

	ResourceViewHeap cbSrUavHeap;

	TransformBuffer modelBuffers[cThreadCount];
	CameraBuffer cameraBuffers[cThreadCount];

	Camera camera;
	ShaderManager shaders;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	CommandListManager commandLists;
	TaskQueue taskQueue;
};
Scene* pScene = nullptr;

void CreateModelCommand(Graphics& g)
{
	auto& lists = pScene->commandLists.GetCommandList("model_bundles");

	const auto threadCount = pScene->taskQueue.ThreadCount();
	const auto countPerThread = pScene->modelPtrs.size() / threadCount;

	for (auto i = 0; i < threadCount; ++i)
	{
		auto count = countPerThread;
		if (i == threadCount - 1)
		{
			count = pScene->modelPtrs.size() - countPerThread * i;
		}

		const auto start = countPerThread * i;
		const auto end = start + count;

		auto pList = g.CreateCommandList(CommandList::SubmitType::Bundle, 1);
		lists.push_back(pList);

		pList->Open(nullptr);

		auto pNativeList = pList->GraphicsList();

		auto pHeap = pScene->cbSrUavHeap.NativePtr();
		pNativeList->SetDescriptorHeaps(1, &pHeap);
		pNativeList->SetGraphicsRootSignature(pScene->pRootSignature.Get());
		pNativeList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ulonglong lastShader = 0ULL;
		for (auto i = start; i < end; ++i)
		{
			auto pModel = pScene->modelPtrs[i];
			const auto shader = pModel->ShaderHash();
			if (lastShader != shader)
			{
				const auto& name = pScene->shaders.Name(shader);
				pNativeList->SetPipelineState(pScene->pPipelineStates[name].Get());
				lastShader = shader;
			}
			pModel->CreateDrawCommand(pNativeList);
		}

		pList->Close();
	}
}

bool SetupScene(Graphics& g)
{
	pScene->taskQueue.Setup(cThreadCount);

	auto pDevice = g.DevicePtr();
	auto pNativeDevice = pDevice->NativePtr();

	auto& commandListPtrs = pScene->commandLists.CreateCommandLists("main", 0);
	pScene->commandLists.CreateCommandLists("model_bundles", -1);
	pScene->commandLists.CommitExecutionOrders();

	auto pCommandList = g.CreateCommandList(CommandList::SubmitType::Direct, 1);
	commandListPtrs.push_back(pCommandList);

	for (auto& pModel : pScene->modelPtrs)
	{
		pModel = new Model();
	}
	Model* rootModels[] = { pScene->modelPtrs[0] };

	rootModels[0]->Setup(pDevice, "assets/test_anim.fbx");
	rootModels[0]->UpdateSubresources(pCommandList, g.CommandQueuePtr());

	fbx::Animation anim;
	anim.LoadFromFile("assets/test_anim.fbx");

	auto meshCount = 0;
	for (auto pModel : pScene->modelPtrs)
	{
		meshCount += pModel->MeshCount();
	}
	pScene->cbSrUavHeap.CreateHeap(
		pDevice, { HeapDesc::ViewType::CbSrUaView, static_cast<int>(pScene->modelPtrs.size()) * 2 + meshCount });

	for (auto& pModel : pScene->modelPtrs)
	{
		pModel->SetupBuffers(&pScene->cbSrUavHeap);
	}

	for (auto i = 0; i < cModelGridSize; ++i)
	{
		for (auto j = 0; j < cModelGridSize; ++j)
		{
			for (auto k = 0; k < cModelGridSize; ++k)
			{
				const auto index = i * cModelGridSize * cModelGridSize + j * cModelGridSize + k;

				auto pModel = pScene->modelPtrs[index];

				auto t = pModel->TransformPtr();
				//t->SetScaling(0.1f, 0.1f, 0.1f);
				//t->SetRotation(0.0f, pScene->rotateAngle, 0.0f);
				//t->SetTranslation(-2.0f + i * 0.3f, -2.0f + j * 0.3f, -1.0f + k * 0.3f);
				t->UpdateMatrix();
			}
		}
	}

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
				IID_PPV_ARGS(&pScene->pRootSignature)));
	}

	{
		for (auto& pModel : pScene->modelPtrs)
		{
			pScene->shaders.LoadFromModelMaterial(&pModel->FbxModel());
		}

		std::sort(
			pScene->modelPtrs.begin(), pScene->modelPtrs.end(),
			[](const Model* lhs, const Model* rhs) { return lhs->ShaderHash() < rhs->ShaderHash();}
		);

		CD3DX12_RASTERIZER_DESC descRS(D3D12_DEFAULT);
		CD3DX12_BLEND_DESC descBS(D3D12_DEFAULT);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = pScene->pRootSignature.Get();
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
			desc.InputLayout = pScene->shaders.InputLayout(name);
			desc.VS = pScene->shaders.VertexShader(name);
			desc.PS = pScene->shaders.PixelShader(name);

			ID3D12PipelineState* pPso;
			ThrowIfFailed(pNativeDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pPso)));
			pScene->pPipelineStates[name] = pPso;
			pPso->Release();
		}
	}

	pScene->rotateAngle = 0.f;

	CreateModelCommand(g);

	return true;
}

CpuStopwatchBatch sw;

void Calc()
{
	sw.Start(100, "calc");

	//pScene->rotateAngle += 0.01f;

#if false
	for (auto i = 0; i < cModelGridSize * cModelGridSize * cModelGridSize; ++i)
	{
		auto pModel = pScene->modelPtrs[i];

		auto t = pModel->TransformPtr();
		t->SetRotation(0.0f, pScene->rotateAngle, 0.0f);
		t->UpdateMatrix();
	}
#else
	const auto threadCount = pScene->taskQueue.ThreadCount();

	auto& models = pScene->modelPtrs;
	const auto countPerThread = models.size() / threadCount;

	for (auto i = 0; i < threadCount; ++i)
	{
		auto count = countPerThread;
		if (i == threadCount - 1)
		{
			count = pScene->modelPtrs.size() - countPerThread * i;
		}

		const auto start = countPerThread * i;
		const auto end = start + count;

		pScene->taskQueue.Enqueue([i, start, end, &models]()
		{
			for (auto j = start; j < end; ++j)
			{
				auto pModel = models[j];

				auto t = pModel->TransformPtr();
				//t->SetRotation(0.0f, pScene->rotateAngle, 0.0f);
				t->UpdateMatrix();
			}
		});
	}
#endif

	sw.Stop(100);
}

void Draw(Graphics& g, GpuStopwatch* pStopwatch)
{
	sw.Start(200, "all");

	sw.Start(201, "camera");
	{
		auto& c = pScene->camera;

		c.SetPosition({ 10.0f, 5.0f, -10.0f });
		c.SetFocus({ 0.0f, 0.0f, 0.0f });
		c.SetUp({ 0.0f, 1.0f, 0.0f });

		c.SetFovY(DirectX::XM_PIDIV4);
		c.SetAspect(g.ScreenPtr()->AspectRatio());
		c.SetNearPlane(0.1f);
		c.SetFarPlane(1000.0f);

		c.UpdateMatrix();

		for (auto i = 0; i < pScene->taskQueue.ThreadCount(); ++i)
		{
			pScene->cameraBuffers[i].View = c.View();
			pScene->cameraBuffers[i].Proj = c.Proj();
		}
	}
	sw.Stop(201);

	auto pGraphicsList = pScene->commandLists.GetCommandList("main")[0];
	pGraphicsList->Open(nullptr);

	auto pNativeGraphicsList = pGraphicsList->GraphicsList();

	{
		auto heap = pScene->cbSrUavHeap.NativePtr();
		pNativeGraphicsList->SetDescriptorHeaps(1, &heap);
	}
	pStopwatch->Start(g.CommandQueuePtr()->NativePtr(), pNativeGraphicsList);

	sw.Start(210, "RS");
	{
		const auto& screen = g.ScreenPtr()->Desc();
		pScene->viewport = { 0.0f, 0.0f, (float)screen.Width, (float)screen.Height, 0.0f, 1.0f };
		pNativeGraphicsList->RSSetViewports(1, &pScene->viewport);

		pScene->scissorRect = { 0, 0, screen.Width, screen.Height };
		pNativeGraphicsList->RSSetScissorRects(1, &pScene->scissorRect);
	}
	sw.Stop(210);

	sw.Start(220, "OM");
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
	sw.Stop(220);

	sw.Start(225, "wait_calc");
	{
		pScene->taskQueue.WaitAll();
	}
	sw.Stop(225);

	const auto threadCount = pScene->taskQueue.ThreadCount();

	sw.Start(230, "models_cbuffer_update");
	{
		auto& models = pScene->modelPtrs;
		const auto countPerThread = models.size() / threadCount;

		for (auto i = 0; i < threadCount; ++i)
		{
			auto count = countPerThread;
			if (i == threadCount - 1)
			{
				count = pScene->modelPtrs.size() - countPerThread * i;
			}

			const auto start = countPerThread * i;
			const auto end = start + count;

			pScene->taskQueue.Enqueue([i, start, end, &models]()
			{
				for (auto j = start; j < end; ++j)
				{
					auto pModel = models[j];
					pScene->modelBuffers[i].World = pModel->TransformPtr()->Matrix();
					pModel->SetTransform(pScene->modelBuffers[i].World, pScene->cameraBuffers[i]);
				}
			});
		}

		//for (auto pModel : pScene->modelPtrs)
		//{
		//	pScene->modelTransforms[0].World = pModel->TransformPtr()->Matrix();
		//	pModel->SetTransform(pScene->modelTransforms[0]);
		//}
	}
	sw.Stop(230);

	sw.Start(240, "models-draw");
	{
		for (auto pBundle : pScene->commandLists.GetCommandList("model_bundles"))
		{
			//pScene->taskQueue.Enqueue([pNativeGraphicsList, pBundle]()
			//{
				pNativeGraphicsList->ExecuteBundle(pBundle->GraphicsList());
			//});
		}
	}
	sw.Stop(240);

	sw.Start(250, "wait_OM");
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			g.CurrentRenderTargetPtr()->NativePtr(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
		);
		pNativeGraphicsList->ResourceBarrier(1, &barrier);
	}
	sw.Stop(250);

	pStopwatch->Stop();

	sw.Start(260, "close_cmdlist");
	{
		pGraphicsList->Close();
	}
	sw.Stop(260);

	sw.Start(265, "wait_models_cbuffer_update");
	{
		pScene->taskQueue.WaitAll();
	}
	sw.Stop(265);

	sw.Start(270, "submit_cmdlist");
	{
		pScene->commandLists.Execute(g.CommandQueuePtr());
	}
	sw.Stop(270);

	sw.Start(280, "swap");
	{
		g.SwapBuffers(1);
	}
	sw.Stop(280);

	sw.Start(290, "wait_cmdlist");
	{
		g.WaitForCommandExecution();
	}
	sw.Stop(290);

	sw.Stop(200);
	if (sw.DumpAll(200, 60))
	{
		sw.Reset();
	}
}

void ShutdownScene()
{
	for (auto& pModel : pScene->modelPtrs)
	{
		SafeDelete(&pModel);
	}
}

int MainImpl(int, char**)
{
	fbx::Setup();

	Window window;
	window.Setup(GetModuleHandle(nullptr), _TEXT("d3d12test"));

	window.Move(300, 200);
	window.Resize(1280, 720);
	window.Open();

	Graphics graphics;

	window.SetEventHandler(WindowEvent::Resize, [&window, &graphics](auto e)
	{
		graphics.WaitForCommandExecution();

		auto pArg = static_cast<ResizeEventArg*>(e);
		graphics.ResizeScreen(pArg->Width, pArg->Height);
	});

	graphics.Setup(true);

	ScreenContextDesc desc = {};
	desc.BufferCount = cBufferCount;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.Width = cScreenWidth;
	desc.Height = cScreenHeight;
	desc.OutputWindow = window.Handle();
	desc.Windowed = true;

	graphics.ResizeScreen(desc);

	pScene = new Scene();
	SetupScene(graphics);

	CpuStopwatch sw;
	GpuStopwatch gsw;
	gsw.Create(graphics.DevicePtr()->NativePtr(), 1);

	FrameCounter counter(&sw, &gsw);

	window.MessageLoop([&graphics, &counter]()
	{
		counter.CpuWatchPtr()->Start();

		Calc();
		Draw(graphics, counter.GpuWatchPtr());

		counter.CpuWatchPtr()->Stop();
		counter.NextFrame();

		if (counter.CpuTime() > 1000.0)
		{
			const auto frames = counter.FrameCount();
			printf(
				"fps: %d, CPU: %.4f %%, GPU: %.4f %%\n",
				counter.FrameCount(),
				counter.CpuUtilization(60),
				counter.GpuUtilization(60));
			counter.Reset();
		}
	});

	graphics.WaitForCommandExecution();

	ShutdownScene();
	SafeDelete(&pScene);

	window.Close();

	fbx::Shutdown();

	//graphics.DevicePtr()->ReportLiveObjects();

	return 0;
}

int main(int argc, char** argv)
{
	/* FBXSDKがグローバルにメモリを確保するので、以下の 8+16+32 bytes はどうやってもリークする
	{235} normal block at 0x000001CF2E6B3F20, 8 bytes long.
	 Data: <        > 98 C4 12 DF F6 7F 00 00 
	{234} normal block at 0x000001CF2E6B41F0, 16 bytes long.
	 Data: < _j.            > 10 5F 6A 2E CF 01 00 00 00 00 00 00 00 00 00 00 
	{233} normal block at 0x000001CF2E6A5F10, 32 bytes long.
	 Data: < Ak.     ?k.    > F0 41 6B 2E CF 01 00 00 20 3F 6B 2E CF 01 00 00

	 更にモデルを読み込むと以下のメモリがリークする
	 {34493} normal block at 0x0000026FDC195BA0, 3840 bytes long.
	 Data: <   @            > 00 00 00 40 00 00 00 00 00 00 00 00 00 00 00 00
	{34492} normal block at 0x0000026FDC1AD000, 24 bytes long.
	 Data: <    ;    [  o   > 00 00 00 00 3B 00 00 00 A0 5B 19 DC 6F 02 00 00
	*/
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	return MainImpl(argc, argv);
	return 0;
}
