#include "CommandList.h"
#include "common.h"
#include "Device.h"

CommandList::~CommandList() {}

HRESULT CommandList::Create(Device* pDevice, SubmitType type, int bufferCount)
{
	HRESULT result;

	auto pNativeDevice = pDevice->NativePtr();

	allocatorPtrs_.resize(bufferCount);
	for (auto i = 0; i < bufferCount; ++i)
	{
		ComPtr<ID3D12CommandAllocator> pAllocator;
		result = pNativeDevice->CreateCommandAllocator(
			static_cast<D3D12_COMMAND_LIST_TYPE>(type),
			IID_PPV_ARGS(&pAllocator));
		if (FAILED(result))
		{
			return result;
		}

		allocatorPtrs_[i] = std::move(pAllocator);
	}

	result = pNativeDevice->CreateCommandList(
		0,
		static_cast<D3D12_COMMAND_LIST_TYPE>(type),
		allocatorPtrs_[currentAllocatorIndex_].Get(),
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

	result = GraphicsList()->Reset(pAllocator.Get(), pPipelineState);
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
