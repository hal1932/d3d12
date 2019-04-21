#include "GpuFence.h"
#include "common.h"
#include "Device.h"

GpuFence::~GpuFence()
{
	SafeCloseHandle(&fenceEvent_);
	SafeRelease(&pFence_);
}

HRESULT GpuFence::Create(Device* pDevice)
{
	HRESULT result;

	result = pDevice->NativePtr()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence_));
	if (FAILED(result))
	{
		return result;
	}

	fenceEvent_ = CreateEventEx(nullptr, FALSE, 0, EVENT_ALL_ACCESS);
	if (!fenceEvent_)
	{
		return S_FALSE;
	}

	return result;
}

HRESULT GpuFence::WaitForCompletion()
{
	auto result = S_OK;

	if (pFence_->GetCompletedValue() < fenceValue_)
	{
		result = pFence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		if (FAILED(result))
		{
			return result;
		}

		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	return result;
}
