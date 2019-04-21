#pragma once
#include <d3d12.h>

class Device
{
public:
	~Device();

	ID3D12Device* NativePtr() { return pDevice_; }

	bool IsDebugEnabled() { return (pDebug_ != nullptr); }
	void ReportLiveObjects()
	{
		ID3D12DebugDevice* debugInterface;
		if (SUCCEEDED(pDevice_->QueryInterface(&debugInterface)))
		{
			debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			debugInterface->Release();
		}
	}

	HRESULT EnableDebugLayer();
	void Create();

private:
	ID3D12Device* pDevice_ = nullptr;
	ID3D12Debug* pDebug_ = nullptr;
};

