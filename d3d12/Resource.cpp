#include "Resource.h"
#include "common.h"
#include "Device.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "GpuFence.h"
#include "ResourceViewHeap.h"
#include "UpdateSubresources.h"
#include <d3dx12.h>

struct UpdateSubresourceContextData
{
	std::vector<std::unique_ptr<Resource>> intermediateResourcePtrs;

	static UpdateSubresourceContextData* GetData(UpdateSubresourceContext* pContext) {
		return reinterpret_cast<UpdateSubresourceContextData*>(pContext->pData);
	}
};

UpdateSubresourceContext::UpdateSubresourceContext()
{
	pData = new UpdateSubresourceContextData();
}

UpdateSubresourceContext::~UpdateSubresourceContext()
{
	auto ptr = UpdateSubresourceContextData::GetData(this);
	SafeDelete(&ptr);
}


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
	return MappedResource().Map(pResource_.Get(), subresource);
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

UpdateSubresourceContext* Resource::UpdateSubresource(const D3D12_SUBRESOURCE_DATA* pData, CommandList* pCommandList, int subresource, UpdateSubresourceContext* pContext)
{
	return UpdateSubresources(pData, pCommandList, subresource, 1, pContext);
}

UpdateSubresourceContext* Resource::UpdateSubresources(const D3D12_SUBRESOURCE_DATA* pData, CommandList* pCommandList, int firstSubresource, int subresourceCount, UpdateSubresourceContext* pContext)
{
	auto pIntermediate = new Resource();
	UpdateSubresourceContextData::GetData(pContext)->intermediateResourcePtrs.emplace_back(pIntermediate);

	pContext->LastResult = ::UpdateSubresources(pDevice_, pData, firstSubresource, subresourceCount, this, pCommandList, pIntermediate);
	if (FAILED(pContext->LastResult))
	{
		return pContext;
	}

	return pContext;
}

void Resource::SetDescriptorHandleLocation(ResourceViewHeap* pHeap, int handleOffset)
{
	pHeap_ = pHeap;
	descriptorHandleOffset_ = handleOffset;
}

D3D12_CPU_DESCRIPTOR_HANDLE Resource::CpuDescriptorHandle()
{
	return pHeap_->CpuHandle(descriptorHandleOffset_);
}

D3D12_GPU_DESCRIPTOR_HANDLE Resource::GpuDescriptorHandle()
{
	return pHeap_->GpuHandle(descriptorHandleOffset_);
}
