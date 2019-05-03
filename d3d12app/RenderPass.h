#pragma once

class Graphics;
class GpuStopwatch;

class RenderPass
{
public:
	RenderPass() {}
	virtual ~RenderPass() {}

	virtual void SetupCommandList(Device* pDevice, CommandQueue* pCommandQueue) = 0;
	virtual void SetupGpuResources(Device* pDevice) = 0;
	virtual void SetupRenderPipeline(Device* pDevice) = 0;

	virtual void Calc() = 0;

	virtual void OpenDraw() = 0;
	virtual void Draw() = 0;
	virtual void CloseDraw(CommandQueue* pCommandQueue) = 0;
};
