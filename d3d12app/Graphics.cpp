#include "Graphics.h"

HRESULT Graphics::Setup(bool debug)
{
	auto result = S_OK;

	if (debug)
	{
		result = device_.EnableDebugLayer();
		if (FAILED(result))
		{
			return result;
		}
	}

	device_.Create();

	result = commandQueue_.Create(&device_);
	if (FAILED(result))
	{
		return result;
	}

	return result;
}

HRESULT Graphics::ResizeScreen(const ScreenContextDesc& desc)
{
	auto result = S_OK;

	renderTargetPtrs_.clear();
	renderTargetHeap_.Reset();

	depthStencilPtr_.reset();
	depthStencilHeap_.Reset();

	screen_.Reset();
	screen_.Create(&device_, &commandQueue_, desc);

	result = renderTargetHeap_.CreateHeap(&device_, { HeapDesc::ViewType::RenderTargetView, desc.BufferCount });
	if (FAILED(result))
	{
		return result;
	}
	for (auto pResource : renderTargetHeap_.CreateRenderTargetViewFromBackBuffer(&screen_))
	{
		renderTargetPtrs_.push_back(std::unique_ptr<Resource>(pResource));
	}

	result = depthStencilHeap_.CreateHeap(&device_, { HeapDesc::ViewType::DepthStencilView, 1 });
	if (FAILED(result))
	{
		return result;
	}
	depthStencilPtr_ = std::unique_ptr<Resource>(
		depthStencilHeap_.CreateDepthStencilView(
			&screen_,{ desc.Width, desc.Height, DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0 }));

	return result;
}

HRESULT Graphics::ResizeScreen(int width, int height)
{
	auto desc = screen_.Desc();
	desc.Width = width;
	desc.Height = height;
	return ResizeScreen(desc);
}

HRESULT Graphics::CreateCommandList(CommandList** ppOut, CommandList::SubmitType type, int bufferCount)
{
	auto pResult = new CommandList();
	auto result = pResult->Create(&device_, type, bufferCount);
	if (FAILED(result))
	{
		SafeDelete(&pResult);
	}

	*ppOut = pResult;
	return result;
}

void Graphics::SubmitCommand(CommandList* pCommandList)
{
	commandQueue_.Submit(pCommandList);
}

void Graphics::SwapBuffers(int syncInterval)
{
	screen_.SwapBuffers(syncInterval);
}

HRESULT Graphics::WaitForCommandExecution()
{
	auto result = S_OK;

	result = commandQueue_.WaitForExecution();
	if (FAILED(result))
	{
		return result;
	}

	screen_.UpdateFrameIndex();

	return result;
}
