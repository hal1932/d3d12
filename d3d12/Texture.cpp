#include "Texture.h"
#include "common.h"
#include "Device.h"
#include "Resource.h"
#include <DirectXTex.h>


Texture::~Texture()
{
	SafeDelete(&pResource_);
	SafeDelete(&pData_);
}

HRESULT Texture::LoadFromFile(const std::wstring& filepath)
{
	HRESULT result;

	SafeDelete(&pData_);
	pData_ = new DirectX::TexMetadata();

	result = DirectX::GetMetadataFromDDSFile(filepath.c_str(), DirectX::DDS_FLAGS_NONE, *pData_);
	if (FAILED(result))
	{
		SafeDelete(&pData_);
		return result;
	}

	filepath_ = filepath;

	return result;
}

HRESULT Texture::UpdateResources(Device* pDevice)
{
	HRESULT result;

	SafeDelete(&pResource_);

	ID3D12Resource* pNativeResource;
	result = DirectX::CreateTexture(pDevice->NativePtr(), *pData_, &pNativeResource);
	if (FAILED(result))
	{
		return result;
	}

	pResource_ = new Resource(pNativeResource, pDevice);

	return result;
}

HRESULT Texture::UpdateSubresource(CommandList* pCommandList, CommandQueue* pCommandQueue)
{
	HRESULT result;

	DirectX::ScratchImage image;
	result = DirectX::LoadFromDDSFile(filepath_.c_str(), DirectX::DDS_FLAGS_NONE, pData_, image);
	if (FAILED(result))
	{
		return result;
	}

	auto data = new D3D12_SUBRESOURCE_DATA[image.GetImageCount()];
	for (int i = 0; i < image.GetImageCount(); ++i)
	{
		auto& pSubImage = image.GetImages()[i];
		data[i].pData = pSubImage.pixels;
		data[i].RowPitch = pSubImage.rowPitch;
		data[i].SlicePitch = pSubImage.slicePitch;
	}

	result = pResource_->UpdateSubresources(data, pCommandList, pCommandQueue, 0, static_cast<int>(image.GetImageCount()));

	SafeDeleteArray(&data);

	return result;
}
