#pragma once
#include "fbxObject.h"
#include <vector>
#include <Windows.h>

class Device;
class Texture;
class CommandList;
struct UpdateSubresourceContext;

namespace fbx
{
	class Material : public Object<FbxSurfaceMaterial>
	{
	public:
		Material(FbxSurfaceMaterial* pSurfaceMaterial);
		~Material();

		const tstring& Name() { return name_; }
		const tstring& Name() const { return name_; }

		size_t TextureCount() { return pTexturePtrs_->size(); }
		Texture* TexturePtr(size_t index) { return pTexturePtrs_->at(index); }

		HRESULT Setup();
		HRESULT UpdateResources(Device* pDevice);
		UpdateSubresourceContext* UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext);

		Material* CreateReference();

	private:
		bool isReference_ = false;

		tstring name_;
		std::vector<Texture*>* pTexturePtrs_;
	};

}// namespace fbx
