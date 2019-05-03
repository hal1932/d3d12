#pragma once
#include "common.h"
#include "fbxsdk.h"
#include <Windows.h>

class Device;
class Texture;
class CommandList;
struct UpdateSubresourceContext;

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
		UpdateSubresourceContext* UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext);

		Material* CreateReference();

	private:
		bool isReference_ = false;

		tstring name_;
		Texture* pTexture_ = nullptr;
	};

}// namespace fbx
