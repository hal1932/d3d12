#include "ScreenContext.h"

#include "common.h"
#include "Device.h"
#include "CommandQueue.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

ScreenContext::~ScreenContext()
{
	Reset();
}

void ScreenContext::Create(Device* pDevice, CommandQueue* pCommandQueue, const ScreenContextDesc& desc)
{
	DXGI_SWAP_CHAIN_DESC rawDesc = {};

	rawDesc.BufferCount = static_cast<UINT>(desc.BufferCount);
	rawDesc.BufferDesc.Format = desc.Format;
	rawDesc.BufferDesc.Width = static_cast<UINT>(desc.Width);
	rawDesc.BufferDesc.Height = static_cast<UINT>(desc.Height);
	rawDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	rawDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	rawDesc.OutputWindow = desc.OutputWindow;
	rawDesc.SampleDesc.Count = 1;
	rawDesc.Windowed = (desc.Windowed) ? TRUE : FALSE;

	if (rawDesc.BufferDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
	{
		rawDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	auto factoryFlags = 0U;
	if (pDevice->IsDebugEnabled())
	{
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}

	HRESULT result;

	ComPtr<IDXGIFactory4> pFactory;
	result = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&pFactory));
	ThrowIfFailed(result);

	ComPtr<IDXGISwapChain> pTmpSwapChain;
	result = pFactory->CreateSwapChain(pCommandQueue->NativePtr(), &rawDesc, pTmpSwapChain.GetAddressOf());
	ThrowIfFailed(result);

	result = pTmpSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain_));
	ThrowIfFailed(result);

	frameIndex_ = pSwapChain_->GetCurrentBackBufferIndex();

	desc_ = desc;
	pDevice_ = pDevice;
}

void ScreenContext::Reset()
{
	SafeRelease(&pSwapChain_);
	frameIndex_ = 0U;
}

void ScreenContext::UpdateFrameIndex()
{
	frameIndex_ = pSwapChain_->GetCurrentBackBufferIndex();
}

void ScreenContext::SwapBuffers(int syncInterval)
{
	pSwapChain_->Present(syncInterval, 0);
}

HRESULT ScreenContext::GetBackBufferView(UINT index, ID3D12Resource** ppView)
{
	return pSwapChain_->GetBuffer(index, IID_PPV_ARGS(ppView));
}
