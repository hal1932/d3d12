#pragma once
#include "common.h"
#include "fbxsdk.h"
#include <Windows.h>

class Device;
class Texture;
class CommandList;
class CommandQueue;

namespace fbx
{
	class Material
	{
	public:
		Material();
		~Material();

		const tstring& Name() { return name_; }
		const tstring& Name() const { return name_; }

		Texture* TexturePtr() { return pTexture_; }

		HRESULT UpdateResources(FbxGeometry* pGeom, Device* pDevice);
		HRESULT UpdateSubresources(CommandList* pCommandList, CommandQueue* pCommandQueue);

		Material* CreateReference();

	private:
		bool isReference_ = false;

		tstring name_;
		Texture* pTexture_ = nullptr;
	};

}// namespace fbx
