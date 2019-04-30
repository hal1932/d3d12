#include <stdlib.h>
#include <crtdbg.h>

#include <memory>
#include <map>

#include <wrl.h>
#include <Windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <algorithm>

#include <lib.h>
#include "Graphics.h"
#include "Model.h"
#include "GameScene.h"

const int cScreenWidth = 1280;
const int cScreenHeight = 720;
const int cBufferCount = 2;


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

	auto pScene = new GameScene();
	pScene->Setup(graphics);

	CpuStopwatch sw;
	GpuStopwatch gsw;
	gsw.Create(graphics.DevicePtr()->NativePtr(), 1);

	FrameCounter counter(&sw, &gsw);

	window.MessageLoop([&pScene, &graphics, &counter]()
	{
		counter.CpuWatchPtr()->Start();

		pScene->Calc();
		pScene->Draw(graphics, counter.GpuWatchPtr());

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
	*/
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	return MainImpl(argc, argv);
}
