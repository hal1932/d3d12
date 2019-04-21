#pragma once
#include <d3d12.h>

class Device;
class GpuFence;
class CommandList;

class CommandQueue
{
public:
	~CommandQueue();

	ID3D12CommandQueue* NativePtr() { return pCommandQueue_; }

	HRESULT Create(Device* pDevice);

	void Submit(CommandList* pCommandList);

	HRESULT WaitForExecution();

private:
	ID3D12CommandQueue* pCommandQueue_ = nullptr;
	GpuFence* pGpuFence_ = nullptr;
};

