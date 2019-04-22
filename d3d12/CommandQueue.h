#pragma once

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

	HRESULT WaitForExecution();

private:
	ComPtr<ID3D12CommandQueue> pCommandQueue_;
	std::unique_ptr<GpuFence> pGpuFence_;
};

