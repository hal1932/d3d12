#pragma once
#include "common.h"
#include <d3d12.h>

class GpuStopwatch
{
public:
	~GpuStopwatch()
	{
		SafeRelease(&pResource_);
		SafeRelease(&pHeap_);
	}

	HRESULT Create(ID3D12Device* pDevice, int count)
	{
		HRESULT result;

		D3D12_QUERY_HEAP_DESC heapDesc = { D3D12_QUERY_HEAP_TYPE_TIMESTAMP, static_cast<uint>(count) * 2, 0U };
		result = pDevice->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&pHeap_));
		if (FAILED(result))
		{
			return result;
		}

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Width = sizeof(UINT64) * count * 2;
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.SampleDesc.Count = 1;

		D3D12_HEAP_PROPERTIES prop = { D3D12_HEAP_TYPE_READBACK, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };
		result = pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pResource_));
		if (FAILED(result))
		{
			return result;
		}

		return result;
	}

	void Start(ID3D12CommandQueue* pCommandQueue, ID3D12GraphicsCommandList* pCommandList)
	{
		pCommandQueue->GetTimestampFrequency(&frequency_);
		pCommandList->EndQuery(pHeap_, D3D12_QUERY_TYPE_TIMESTAMP, 0);
		pCommandList_ = pCommandList;

		isQueried_ = true;
	}

	void Stop()
	{
		// TODO: EndQueryがパイプラインに載らないようにする
		// このままだとEndQueryが他のコマンドと同時実行されて計測結果がブレる可能性があるけど、
		// Fenceを挟むにはいったんCommandList::Close()してからQueue::Signal()しなきゃいけないから、
		// どう実装すれば綺麗にまとまるか悩みどころ
		pCommandList_->EndQuery(pHeap_, D3D12_QUERY_TYPE_TIMESTAMP, 1);
		pCommandList_->ResolveQueryData(pHeap_, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, pResource_, 0);
		pCommandList_ = nullptr;
	}

	void Reset() {}

	double ElaspedMilliseconds()
	{
		if (!isQueried_)
		{
			return 0.0;
		}

		D3D12_RANGE range = { 0, sizeof(UINT64) * 2 };
		void* ptr = nullptr;
		pResource_->Map(0, &range, &ptr);

		const auto data = reinterpret_cast<ulonglong*>(ptr);
		const auto begin = data[0];
		const auto end = data[1];

		pResource_->Unmap(0, nullptr);

		return (end - begin) * 1000.0 / frequency_;
	}

private:
	ID3D12GraphicsCommandList* pCommandList_;

	ID3D12QueryHeap* pHeap_;
	ID3D12Resource* pResource_;

	bool isQueried_ = false;

	UINT64 frequency_;
	UINT64 fenceValue_ = 0;
};

