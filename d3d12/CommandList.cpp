#include "CommandList.h"
#include "common.h"
#include "Device.h"

CommandList::~CommandList()
{
	SafeRelease(&pNativeList_);

	for (auto pAllocator : allocatorPtrs_)
	{
		SafeRelease(&pAllocator);
	}
}

HRESULT CommandList::Create(Device* pDevice, SubmitType type, int bufferCount)
{
	HRESULT result;

	auto pNativeDevice = pDevice->NativePtr();

	allocatorPtrs_.resize(bufferCount);
	for (auto i = 0; i < bufferCount; ++i)
	{
		ID3D12CommandAllocator* pAllocator;
		result = pNativeDevice->CreateCommandAllocator(
			static_cast<D3D12_COMMAND_LIST_TYPE>(type),
			IID_PPV_ARGS(&pAllocator));
		if (FAILED(result))
		{
			return result;
		}

		allocatorPtrs_[i] = pAllocator;
	}

	result = pNativeDevice->CreateCommandList(
		0,
		static_cast<D3D12_COMMAND_LIST_TYPE>(type),
		allocatorPtrs_[currentAllocatorIndex_],
		nullptr,
		IID_PPV_ARGS(&pNativeList_));
	if (FAILED(result))
	{
		return result;
	}

	Close();

	type_ = type;
	allocatorCount_ = bufferCount;

	return result;
}

HRESULT CommandList::Open(ID3D12PipelineState* pPipelineState, bool swapBuffers)
{
	auto result = S_OK;

	auto pAllocator = allocatorPtrs_[currentAllocatorIndex_];
	
	result = pAllocator->Reset();
	if (FAILED(result))
	{
		return result;
	}

	result = GraphicsList()->Reset(pAllocator, pPipelineState);
	if (FAILED(result))
	{
		return result;
	}

	if (swapBuffers)
	{
		if (++currentAllocatorIndex_ >= allocatorCount_)
		{
			currentAllocatorIndex_ = 0;
		}
	}

	return result;
}

void CommandList::Close()
{
	GraphicsList()->Close();
}
