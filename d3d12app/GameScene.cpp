#include "GameScene.h"
#include "Graphics.h"
#include <cmath>
#include <set>


GameScene::~GameScene() {}


void GameScene::Setup(Graphics& g)
{
	taskQueue_.Setup(cThreadCount);
	models_.Resize(cModelCount);

	auto pDevice = g.DevicePtr();

	auto resourceViewCount = 0;
	{
		auto pCommandList = std::make_unique<CommandList>();
		pCommandList->Create(pDevice, CommandList::SubmitType::Direct, 1);
		pCommandList->Open(nullptr, false);

		UpdateSubresourceContext context;
		for (auto& model : models_)
		{
			model.LoadFromFile("assets/test_anim.fbx");
			model.UpdateResources(pDevice);
			model.UpdateSubresources(pCommandList.get(), &context);
			resourceViewCount += model.ResourceBufferCount();
		}

		pCommandList->Close();
		g.SubmitCommand(pCommandList.get());
		g.WaitForCommandExecution();
	}

	cbSrUavHeap_.CreateHeap(pDevice, { HeapDesc::ViewType::CbSrUaView, resourceViewCount + 1 });
	for (auto& model : models_)
	{
		model.SetupBuffers(&cbSrUavHeap_);
	}
	cameraCbv_.Setup(&cbSrUavHeap_);

	modelPass_.SetModels(&models_, &cbSrUavHeap_);
	modelPass_.SetCamera(&cameraCbv_);
	modelPass_.SetTaskQueue(&taskQueue_);
	modelPass_.SetScreenSize(g.ScreenPtr()->Width(), g.ScreenPtr()->Height());
	modelPass_.SetupCommandList(pDevice, g.CommandQueuePtr());

	//modelPass_.SetupGpuResources(pDevice);
	modelPass_.SetupRenderPipeline(pDevice);

	for (auto i = 0; i < models_.Size(); ++i)
	{
		models_[i].SetAnimCurrentFrame(i * 5);
	}
}


void GameScene::Calc(Graphics& g)
{
	cpuTimer_.Start(100, "calc");

	const auto threadCount = taskQueue_.ThreadCount();

	auto& models = models_;
	const auto countPerThread = models_.Size() / threadCount;

	for (auto i = 0; i < threadCount; ++i)
	{
		auto count = countPerThread;
		if (i == threadCount - 1)
		{
			count = models.Size() - countPerThread * i;
		}

		const auto start = countPerThread * i;
		const auto end = start + count;

		const auto frameIndex = frameIndex_;

		taskQueue_.Enqueue([start, end, frameIndex, &models]()
		{
			for (auto j = start; j < end; ++j)
			{
				const auto x = -2.0f + 4.0f * static_cast<float>((j / cModelGridSize) % cModelGridSize);
				const auto y = -2.0f + 4.0f * static_cast<float>(j % cModelGridSize);
				const auto z = -2.0f + 4.0f * static_cast<float>((j / (cModelGridSize * cModelGridSize)) % cModelGridSize);
				const auto s = 0.25f;

				auto t = models[j].TransformPtr();
				t->SetTranslation(x, y, z);
				t->SetScaling(s, s, s);
				t->UpdateMatrix();
			}
		});
	}

	{
		auto& c = camera_;

		//c.SetPosition({ 10.0f, 5.0f, -10.0f + std::sinf(static_cast<float>(frameIndex_) / 10.0f) * 10.0f });
		c.SetPosition({ 10.0f, 5.0f, -10.0f });
		c.SetFocus({ 0.0f, 0.0f, 0.0f });
		c.SetUp({ 0.0f, 1.0f, 0.0f });

		c.SetFovY(DirectX::XM_PIDIV4);
		c.SetAspect(g.ScreenPtr()->AspectRatio());
		c.SetNearPlane(0.1f);
		c.SetFarPlane(1000.0f);

		c.UpdateMatrix();
	}

	modelPass_.Calc();

	++frameIndex_;

	cpuTimer_.Stop(100);
}


void GameScene::Draw(Graphics& g, GpuStopwatch* pStopwatch)
{
	cpuTimer_.Start(200, "all");

	cpuTimer_.Start(201, "camera");
	{
		auto& c = camera_;

		CameraConstant cb;
		cb.View = c.View();
		cb.Proj = c.Proj();

		cameraCbv_.CopyBufferFrom(cb);
	}
	cpuTimer_.Stop(201);

	Resource* renderTargets[] = { g.CurrentRenderTargetPtr() };
	modelPass_.SetRenderTargets(1, renderTargets, g.DepthStencilPtr());

	modelPass_.OpenDraw();

	cpuTimer_.Start(225, "wait_calc");
	{
		taskQueue_.WaitAll();
	}
	cpuTimer_.Stop(225);

	const auto threadCount = taskQueue_.ThreadCount();

	cpuTimer_.Start(230, "models_cbuffer_update");
	{
		const auto countPerThread = models_.Size() / threadCount;

		for (auto i = 0; i < threadCount; ++i)
		{
			auto count = countPerThread;
			if (i == threadCount - 1)
			{
				count = models_.Size() - countPerThread * i;
			}

			const auto start = countPerThread * i;
			const auto end = start + count;

			taskQueue_.Enqueue([i, start, end, this]()
			{
				fbx::TransformConstant cb;
				for (auto j = start; j < end; ++j)
				{
					auto& modle = models_[j];
					cb.World = modle.TransformPtr()->Matrix();
					modle.SetTransform(cb.World);
				}
			});
		}
	}
	cpuTimer_.Stop(230);

	modelPass_.Draw();

	cpuTimer_.Start(265, "wait_models_cbuffer_update");
	{
		taskQueue_.WaitAll();
	}
	cpuTimer_.Stop(265);

	modelPass_.CloseDraw(g.CommandQueuePtr());

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
