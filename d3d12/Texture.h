#pragma once
#include "Windows.h"
#include "Resource.h"
#include <DirectXTex.h>

namespace DirectX
{
	struct TexMetadata;
}

class Device;
class Resource;
class CommandList;
class CommandQueue;

class Texture
{
public:
	~Texture();

	Resource* ResourcePtr() { return pResource_.get(); }

	HRESULT LoadFromFile(const std::wstring& filepath);
	HRESULT UpdateResources(Device* pDevice);
	HRESULT UpdateSubresource(CommandList* pCommandList, CommandQueue* pCommandQueue);

private:
	std::unique_ptr<DirectX::TexMetadata> pData_ = nullptr;
	std::wstring filepath_;
	std::unique_ptr<Resource> pResource_;
};

