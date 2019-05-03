#include "Texture.h"
#include "common.h"
#include "Device.h"



Texture::~Texture() {}

HRESULT Texture::LoadFromFile(const std::wstring& filepath)
{
	HRESULT result;

	pData_.reset();
	pData_ = std::make_unique<DirectX::TexMetadata>();

	result = DirectX::GetMetadataFromDDSFile(filepath.c_str(), DirectX::DDS_FLAGS_NONE, *pData_);
	if (FAILED(result))
	{
		return result;
	}

	filepath_ = filepath;

	return result;
}

HRESULT Texture::UpdateResources(Device* pDevice)
{
	HRESULT result;

	pResource_.reset();

	ID3D12Resource* pNativeResource;
	result = DirectX::CreateTexture(pDevice->NativePtr(), *pData_, &pNativeResource);
	if (FAILED(result))
	{
		return result;
	}

	pResource_ = std::make_unique<Resource>(pNativeResource, pDevice);

	return result;
}

UpdateSubresourceContext* Texture::UpdateSubresource(CommandList* pCommandList, UpdateSubresourceContext* pContext)
{
	DirectX::ScratchImage image;
	pContext->LastResult = DirectX::LoadFromDDSFile(filepath_.c_str(), DirectX::DDS_FLAGS_NONE, pData_.get(), image);
	if (FAILED(pContext->LastResult))
	{
		return pContext;
	}

	auto data = std::make_unique<D3D12_SUBRESOURCE_DATA[]>(image.GetImageCount());
	for (int i = 0; i < image.GetImageCount(); ++i)
	{
		auto& pSubImage = image.GetImages()[i];
		data[i].pData = pSubImage.pixels;
		data[i].RowPitch = pSubImage.rowPitch;
		data[i].SlicePitch = pSubImage.slicePitch;
	}

	return pResource_->UpdateSubresources(data.get(), pCommandList, 0, static_cast<int>(image.GetImageCount()), pContext);
}
