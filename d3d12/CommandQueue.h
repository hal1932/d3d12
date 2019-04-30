#pragma once
#include "GpuFence.h"

class Device;
class GpuFence;
class CommandList;

class CommandQueue
{
public:
	~CommandQueue();

	ID3D12CommandQueue* NativePtr() { return pCommandQueue_.Get(); }

	HRESULT Create(Device* pDevice);

	void Submit(CommandList* pCommandList);

	HRESULT Signal(GpuFence* pGpuFence) {
		return Signal(pGpuFence, pGpuFence->CurrentValue());
	}
	HRESULT Signal(GpuFence* pGpuFence, UINT64 fenceValue);
	
	HRESULT Wait(GpuFence* pGpuFence) {
		return Wait(pGpuFence, pGpuFence->CurrentValue());
	}
	HRESULT Wait(GpuFence* pGpuFence, UINT64 fenceValue);

	HRESULT WaitForExecution();

private:
	ComPtr<ID3D12CommandQueue> pCommandQueue_;
	std::unique_ptr<GpuFence> pGpuFence_;
};

