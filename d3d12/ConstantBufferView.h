#pragma once
#include "ResourceViewHeap.h"
#include "Resource.h"

template<class T>
class ConstantBufferView
{
public:
	~ConstantBufferView()
	{
		if (buffer_ != nullptr)
		{
			pResource_->Unmap(0);
		}
	}

	Resource* ResourcePtr() { return pResource_.get(); }

	void Setup(ResourceViewHeap* pHeap)
	{
		CsvDesc csvDesc = { sizeof(T), D3D12_TEXTURE_LAYOUT_ROW_MAJOR };
		pResource_ = std::unique_ptr<Resource>(pHeap->CreateConstantBufferView(csvDesc));
		buffer_ = pResource_->Map(0);
	}

	void CopyBufferFrom(const T& data)
	{
		memcpy(buffer_, &data, sizeof(T));
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GpuDescriptorHandle()
	{
		return pResource_->GpuDescriptorHandle();
	}

private:
	std::unique_ptr<Resource> pResource_;
	void* buffer_ = nullptr;
};

