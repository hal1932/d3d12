#include "Device.h"
#include "common.h"


Device::~Device()
{
	if (IsDebugEnabled())
	{
		ID3D12DebugDevice* debugInterface;
		if (SUCCEEDED(pDevice_->QueryInterface(&debugInterface)))
		{
			debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			debugInterface->Release();
		}
	}
}

HRESULT Device::EnableDebugLayer()
{
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug_));
	if (SUCCEEDED(result))
	{
		pDebug_->EnableDebugLayer();
	}
	return result;
}

void Device::Create()
{
	auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice_));
	ThrowIfFailed(result);
}
