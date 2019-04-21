#pragma once
#include "ResourceViewHeap.h"
#include "Resource.h"
#include <memory>

template<class T>
class ConstantBuffer
{
public:
	~ConstantBuffer()
	{
		if (buffer_ != nullptr)
		{
			resourcePtr_->Unmap(0);
		}
	}

	void Setup(ResourceViewHeap* pHeap)
	{
		CsvDesc csvDesc = { sizeof(T), D3D12_TEXTURE_LAYOUT_ROW_MAJOR };
		resourcePtr_ = std::unique_ptr<Resource>(pHeap->CreateConstantBufferView(csvDesc));
		buffer_ = resourcePtr_->Map(0);
	}

	void SetBuffer(const T& data)
	{
		memcpy(buffer_, &data, sizeof(T));
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GpuDescriptorHandle()
	{
		return resourcePtr_->GpuDescriptorHandle();
	}

private:
	std::unique_ptr<Resource> resourcePtr_;
	void* buffer_ = nullptr;
};

