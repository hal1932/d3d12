#pragma once

class Device;

class GpuFence
{
public:
	~GpuFence();

	ID3D12Fence* NativePtr() { return pFence_.Get(); }
	UINT64 CurrentValue() { return fenceValue_; }

	HRESULT Create(Device* pDevice);
	void IncrementValue(){ ++fenceValue_; }
	HRESULT WaitForCompletion();

private:
	ComPtr<ID3D12Fence> pFence_;
	UINT64 fenceValue_ = 0ULL;
	HANDLE fenceEvent_ = nullptr;
};

