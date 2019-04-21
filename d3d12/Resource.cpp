#include "Resource.h"
#include "common.h"
#include "Device.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "GpuFence.h"
#include "ResourceViewHeap.h"
#include "UpdateSubresources.h"
#include <d3dx12.h>

Resource::Resource(ID3D12Resource* pResource, Device* pDevice)
	: pDevice_(pDevice),
	pResource_(pResource)
{
	D3D12_HEAP_PROPERTIES heapProp;
	D3D12_HEAP_FLAGS heapFlags;
	pResource->GetHeapProperties(&heapProp, &heapFlags);

	const auto desc = pResource->GetDesc();

	desc_.HeapType = heapProp.Type;
	desc_.Format = desc.Format;
	desc_.Dimension = desc.Dimension;
	desc_.Width = static_cast<int>(desc.Width);
	desc_.Height = desc.Height;
	desc_.Depth = desc.DepthOrArraySize;
	desc_.MipLevels = desc.MipLevels;
	desc_.SampleCount = desc.SampleDesc.Count;
	desc_.Layout = desc.Layout;
	desc_.Flags = desc.Flags;
}

Resource::~Resource()
{
	SafeRelease(&pResource_);
	pDevice_ = nullptr;
}

HRESULT Resource::CreateCommited(Device* pDevice, const ResourceDesc& desc)
{
	return CreateCommitedImpl_(pDevice, desc, nullptr);
}

HRESULT Resource::CreateCommited(Device* pDevice, const ResourceDesc& desc, const D3D12_CLEAR_VALUE& clearValue)
{
	return CreateCommitedImpl_(pDevice, desc, &clearValue);
}

HRESULT Resource::CreateCommitedImpl_(Device* pDevice, const ResourceDesc& desc, const D3D12_CLEAR_VALUE* pClearValue)
{
	CD3DX12_HEAP_PROPERTIES heapProp(desc.HeapType);

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = desc.Dimension;
	resDesc.Format = desc.Format;
	resDesc.Width = desc.Width;
	resDesc.Height = desc.Height;
	resDesc.DepthOrArraySize = static_cast<UINT16>(desc.Depth);
	resDesc.MipLevels = static_cast<UINT16>(desc.MipLevels);
	resDesc.SampleDesc.Count = static_cast<UINT>(desc.SampleCount);
	resDesc.Flags = desc.Flags;
	resDesc.Layout = desc.Layout;


	HRESULT result;

	auto pNativeDevice = pDevice->NativePtr();

	result = pNativeDevice->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, desc.States,
		pClearValue,
		IID_PPV_ARGS(&pResource_));
	if (FAILED(result))
	{
		return result;
	}

	pDevice_ = pDevice;
	desc_ = desc;

	return result;
}

HRESULT Resource::CreateVertexBuffer(Device* pDevice, int size)
{
	const auto desc = ResourceDesc::Buffer(
		D3D12_RESOURCE_STATE_GENERIC_READ,
		size, D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
	return CreateCommited(pDevice, desc);
}

HRESULT Resource::CreateIndexBuffer(Device* pDevice, int size)
{
	return CreateVertexBuffer(pDevice, size);
}

void* Resource::Map(int subresource)
{
	void* pData;
	auto result = pResource_->Map(subresource, nullptr, &pData);
	return (SUCCEEDED(result)) ? pData : nullptr;
}

void Resource::Unmap(int subresource)
{
	pResource_->Unmap(subresource, nullptr);
}

MappedResource Resource::ScopedMap(int subresource)
{
	return MappedResource().Map(pResource_, subresource);
}

D3D12_VERTEX_BUFFER_VIEW Resource::GetVertexBufferView(int stride)
{
	return
	{
		pResource_->GetGPUVirtualAddress(),
		static_cast<UINT>(desc_.Width),
		static_cast<UINT>(stride)
	};
}

D3D12_INDEX_BUFFER_VIEW Resource::GetIndexBufferView(DXGI_FORMAT format)
{
	return
	{
		pResource_->GetGPUVirtualAddress(),
		static_cast<UINT>(desc_.Width),
		format
	};
}

HRESULT Resource::UpdateSubresource(const D3D12_SUBRESOURCE_DATA* pData, CommandList* pCommandList, CommandQueue* pCommandQueue, int subresource)
{
	return UpdateSubresources(pData, pCommandList, pCommandQueue, subresource, 1);
}

HRESULT Resource::UpdateSubresources(const D3D12_SUBRESOURCE_DATA* pData, CommandList* pCommandList, CommandQueue* pCommandQueue, int firstSubresource, int subresourceCount)
{
	HRESULT result;

	Resource intermediate;
	result = ::UpdateSubresources(pDevice_, pData, firstSubresource, subresourceCount, this, pCommandList, &intermediate);
	if (FAILED(result))
	{
		return result;
	}

	pCommandQueue->Submit(pCommandList);
	result = pCommandQueue->WaitForExecution();

	return result;
}

D3D12_CPU_DESCRIPTOR_HANDLE Resource::CpuDescriptorHandle()
{
	if (pHeap_)
	{
		return pHeap_->CpuHandle(descriptorHandleIndex_);
	}
	return { 0 };
}

D3D12_GPU_DESCRIPTOR_HANDLE Resource::GpuDescriptorHandle()
{
	if (pHeap_)
	{
		return pHeap_->GpuHandle(descriptorHandleIndex_);
	}
	return{ 0 };
}

void Resource::SetResourceViewHeap(ResourceViewHeap* pHeap, int descriptorHandleIndex)
{
	pHeap_ = pHeap;
	descriptorHandleIndex_ = descriptorHandleIndex;
}
