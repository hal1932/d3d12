#pragma once
#include "ResourceDesc.h"

class Device;
class CommandList;
class CommandQueue;
class GpuFence;
class ResourceViewHeap;

class MappedResource
{
public:
	MappedResource()
		: pResource_(nullptr)
	{}

	MappedResource(MappedResource& other)
		: pData_(other.pData_),
		pResource_(other.pResource_),
		subresource_(other.subresource_)
	{
		other.pData_ = nullptr;
		other.pResource_ = nullptr;
	}

	~MappedResource()
	{
		if (pData_ != nullptr)
		{
			pResource_->Unmap(subresource_, nullptr);
		}
	}

	MappedResource& Map(ID3D12Resource* pResource, int subresource)
	{
		auto result = pResource->Map(subresource, nullptr, &pData_);
		if (FAILED(result))
		{
			pData_ = nullptr;
		}

		pResource_ = pResource;
		subresource_ = subresource;

		return *this;
	}

	void* NativePtr() { return pData_; }

private:
	void* pData_;
	ID3D12Resource* pResource_;
	int subresource_;
};

class Resource
{
public:
	Resource() {}
	Resource(ID3D12Resource* pResource, Device* pDevice);
	~Resource();

	ID3D12Resource* NativePtr() { return pResource_.Get(); }

	HRESULT CreateCommited(Device* pDevice, const ResourceDesc& desc);
	HRESULT CreateCommited(Device* pDevice, const ResourceDesc& desc, const D3D12_CLEAR_VALUE& clearValue);

	HRESULT CreateVertexBuffer(Device* pDevice, int size);
	HRESULT CreateIndexBuffer(Device* pDevice, int size);

	void* Map(int subresource);
	void Unmap(int subresource);

	MappedResource ScopedMap(int subresource);

	HRESULT UpdateSubresource(const D3D12_SUBRESOURCE_DATA* pData, CommandList* pCommandList, CommandQueue* pCommandQueue, int subresource);
	HRESULT UpdateSubresources(const D3D12_SUBRESOURCE_DATA* pData, CommandList* pCommandList, CommandQueue* pCommandQueue, int firstSubresource, int subresourceCount);

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(int stride);
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(DXGI_FORMAT format);

	D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptorHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GpuDescriptorHandle();

	void SetResourceViewHeap(ResourceViewHeap* pHeap, int descriptorHandleIndex);

private:
	Device* pDevice_ = nullptr;
	ComPtr<ID3D12Resource> pResource_;
	ResourceDesc desc_;

	ResourceViewHeap* pHeap_ = nullptr;
	int descriptorHandleIndex_ = -1;

	HRESULT CreateCommitedImpl_(Device* pDevice, const ResourceDesc& desc, const D3D12_CLEAR_VALUE* pClearValue);
};
