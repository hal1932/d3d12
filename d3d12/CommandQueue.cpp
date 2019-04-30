#include "CommandQueue.h"
#include "common.h"
#include "Device.h"
#include "GpuFence.h"
#include "CommandList.h"

CommandQueue::~CommandQueue() {}

HRESULT CommandQueue::Create(Device* pDevice)
{
	D3D12_COMMAND_QUEUE_DESC rawDesc = {};
	rawDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	rawDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT result;

	result = pDevice->NativePtr()->CreateCommandQueue(&rawDesc, IID_PPV_ARGS(&pCommandQueue_));
	if (FAILED(result))
	{
		return result;
	}

	pGpuFence_ = std::make_unique<GpuFence>();

	result = pGpuFence_->Create(pDevice);
	if (FAILED(result))
	{
		return result;
	}

	return result;
}

void CommandQueue::Submit(CommandList* pCommandList)
{
	ID3D12CommandList* ppCmdLists[] = { pCommandList->GraphicsList() };
	pCommandQueue_->ExecuteCommandLists(1, ppCmdLists);
}

HRESULT CommandQueue::Signal(GpuFence* pGpuFence, UINT64 fenceValue)
{
	if (pGpuFence->CompletedValue() < fenceValue)
	{
		return pCommandQueue_->Signal(pGpuFence->NativePtr(), fenceValue);
	}
	return S_OK;
}

HRESULT CommandQueue::Wait(GpuFence* pGpuFence, UINT64 fenceValue)
{
	if (pGpuFence->CompletedValue() < fenceValue)
	{
		return pCommandQueue_->Wait(pGpuFence->NativePtr(), fenceValue);
	}
	return S_OK;
}

HRESULT CommandQueue::WaitForExecution()
{
	HRESULT result;

	pGpuFence_->IncrementValue();

	result = pCommandQueue_->Signal(pGpuFence_->NativePtr(), pGpuFence_->CurrentValue());
	if (FAILED(result))
	{
		return result;
	}

	result = pGpuFence_->WaitForCompletion();
	if (FAILED(result))
	{
		return result;
	}

	return result;
}
