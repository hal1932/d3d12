#include "ResourceViewHeap.h"
#include "common.h"
#include "Device.h"
#include "ScreenContext.h"
#include "Resource.h"
#include "Texture.h"

ResourceViewHeap::~ResourceViewHeap()
{
	Reset();
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceViewHeap::CpuHandle(int index)
{
	auto handle = pDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += index * descriptorSize_;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ResourceViewHeap::GpuHandle(int index)
{
	auto handle = pDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += index * descriptorSize_;
	return handle;
}

HRESULT ResourceViewHeap::CreateHeap(Device* pDevice, const HeapDesc& desc)
{
	switch (desc.ViewType)
	{
		case HeapDesc::ViewType::RenderTargetView:
			return CreateHeapImpl_(pDevice, desc, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		case HeapDesc::ViewType::DepthStencilView:
			return CreateHeapImpl_(pDevice, desc, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		case HeapDesc::ViewType::CbSrUaView:
			return CreateHeapImpl_(pDevice, desc, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		default:
			return S_FALSE;
	}
}

void ResourceViewHeap::Reset()
{
	pDescriptorHeap_.Reset();
	descriptorSize_ = 0U;
	resourceCount_ = 0U;
	currentSize_ = 0;
}

std::vector<Resource*> ResourceViewHeap::CreateRenderTargetViewFromBackBuffer(ScreenContext* pScreen)
{
	D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
	viewDesc.Format = pScreen->Desc().Format;
	viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	HRESULT result;
	std::vector<Resource*> resourcePtrs;

	auto pNativeDevice = pDevice_->NativePtr();

	const auto handleOffset = static_cast<int>(currentSize_);
	auto handle = CpuHandle(handleOffset);
	for (int i = handleOffset; i < pScreen->Desc().BufferCount; ++i)
	{
		ID3D12Resource* pView;
		result = pScreen->GetBackBufferView(i, &pView);
		if (FAILED(result))
		{
			return resourcePtrs;
		}

		auto pResource = new Resource(pView, pDevice_);

		pNativeDevice->CreateRenderTargetView(pView, &viewDesc, handle);
		pResource->SetDescriptorHandleLocation(this, i);
		resourcePtrs.push_back(pResource);
		
		handle.ptr += descriptorSize_;

		++currentSize_;
	}

	return resourcePtrs;
}

Resource* ResourceViewHeap::CreateDepthStencilView(ScreenContext* pContext, const DsvDesc& desc)
{
	auto resourceDesc = ResourceDesc::Tex2D(
		desc.Format, D3D12_RESOURCE_STATE_DEPTH_WRITE,
		desc.Width, desc.Height);
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = desc.Format;
	clearValue.DepthStencil.Depth = desc.ClearDepth;
	clearValue.DepthStencil.Stencil = desc.ClearStencil;

	HRESULT result;

	auto pNativeDevice = pDevice_->NativePtr();

	auto pResource = new Resource();
	result = pResource->CreateCommited(pDevice_, resourceDesc, clearValue);
	if (FAILED(result))
	{
		return nullptr;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
	viewDesc.Format = desc.Format;
	viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	auto handle = CpuHandle(currentSize_);
	pNativeDevice->CreateDepthStencilView(pResource->NativePtr(), &viewDesc, handle);
	pResource->SetDescriptorHandleLocation(this, currentSize_);

	++currentSize_;

	return pResource;
}

Resource* ResourceViewHeap::CreateConstantBufferView(const CsvDesc& desc)
{
	const auto resourceDesc = ResourceDesc::Buffer(
		D3D12_RESOURCE_STATE_GENERIC_READ,
		desc.Size, desc.Layout);

	HRESULT result;

	auto pNativeDevice = pDevice_->NativePtr();

	auto pResource = new Resource();
	result = pResource->CreateCommited(pDevice_, resourceDesc);
	if (FAILED(result))
	{
		return nullptr;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
	viewDesc.BufferLocation = pResource->NativePtr()->GetGPUVirtualAddress();
	viewDesc.SizeInBytes = static_cast<UINT>(desc.Size);

	auto handle = CpuHandle(currentSize_);
	pNativeDevice->CreateConstantBufferView(&viewDesc, handle);
	pResource->SetDescriptorHandleLocation(this, currentSize_);

	++currentSize_;

	return pResource;
}

Resource* ResourceViewHeap::CreateShaderResourceView(const SrvDesc& desc)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.ViewDimension = desc.Dimension;

	Resource* pResource = nullptr;

	switch (desc.Dimension)
	{
		case D3D12_SRV_DIMENSION_TEXTURE2D:
		{
			const auto texDesc = desc.pTexture->ResourcePtr()->NativePtr()->GetDesc();

			// ‚Æ‚è‚ ‚¦‚¸RGBA‚ðŽw’è‚µ‚Ä‚¨‚­
			viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			viewDesc.Format = texDesc.Format;
			viewDesc.Texture2D.MipLevels = texDesc.MipLevels;
			viewDesc.Texture1D.MostDetailedMip = 0;

			pResource = desc.pTexture->ResourcePtr();
			pResource->SetDescriptorHandleLocation(this, currentSize_);

			break;
		}
	}

	auto pNativeDevice = pDevice_->NativePtr();

	auto handle = CpuHandle(currentSize_);
	pNativeDevice->CreateShaderResourceView(pResource->NativePtr(), &viewDesc, handle);

	++currentSize_;

	return pResource;
}

HRESULT ResourceViewHeap::CreateHeapImpl_(
	Device* pDevice,
	const HeapDesc& desc,
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = static_cast<UINT>(desc.BufferCount);
	heapDesc.Type = type;
	heapDesc.Flags = flags;

	HRESULT result;

	auto pNativeDevice = pDevice->NativePtr();

	result = pNativeDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pDescriptorHeap_));
	if (FAILED(result))
	{
		return result;
	}

	pDevice_ = pDevice;
	descriptorSize_ = pNativeDevice->GetDescriptorHandleIncrementSize(heapDesc.Type);
	resourceCount_ = heapDesc.NumDescriptors;

	return result;
}
