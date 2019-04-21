#pragma once
#include <lib.h>

class Graphics
{
public:
	HRESULT Setup(bool debug);

	Device* DevicePtr() { return &device_; }
	ScreenContext* ScreenPtr() { return &screen_; }
	CommandQueue* CommandQueuePtr() { return &commandQueue_; }

	Resource* CurrentRenderTargetPtr() { return renderTargetPtrs_[screen_.FrameIndex()].get(); }
	Resource* DepthStencilPtr() { return depthStencilPtr_.get(); }

	HRESULT ResizeScreen(const ScreenContextDesc& desc);
	HRESULT ResizeScreen(int width, int height);

	HRESULT CreateCommandList(CommandList** ppOut, CommandList::SubmitType type, int bufferCount);

	CommandList* CreateCommandList(CommandList::SubmitType type, int bufferCount)
	{
		CommandList* pOut = nullptr;
		CreateCommandList(&pOut, type, bufferCount);
		return pOut;
	}

	void SubmitCommand(CommandList* pCommandList);
	void SwapBuffers(int syncInterval);
	HRESULT WaitForCommandExecution();

private:
	Device device_;
	ScreenContext screen_;

	ResourceViewHeap renderTargetHeap_;
	std::vector<std::unique_ptr<Resource>> renderTargetPtrs_;

	ResourceViewHeap depthStencilHeap_;
	std::unique_ptr<Resource> depthStencilPtr_;

	CommandQueue commandQueue_;
};

