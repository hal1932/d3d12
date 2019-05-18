#include "fbxMaterial.h"
#include "common.h"
#include "Device.h"
#include "Texture.h"
#include <iostream>

using namespace fbx;

Material::Material(FbxSurfaceMaterial* pSurfaceMaterial)
	: Object(pSurfaceMaterial)
{}

Material::~Material()
{
	if (!isReference_)
	{
		SafeDeleteSequence(pTexturePtrs_);
		SafeDelete(&pTexturePtrs_);
	}
}

HRESULT Material::Setup()
{
	auto pMaterial = NativePtr();

	name_ = pMaterial->GetName();

	// とりあえずランバートだけ対応
	if (pMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		auto pLambert = static_cast<FbxSurfaceLambert*>(pMaterial);
		//for (int j = 0; j < 3; ++j)
		//{
		//	std::cout << pLambert->Diffuse.Get()[j] << " ";
		//}
		//std::cout << std::endl;
	}

	// テクスチャ
	pTexturePtrs_ = new std::vector<Texture*>();
	{
		int layerIndex;
		FBXSDK_FOR_EACH_TEXTURE(layerIndex)
		{
			auto prop = pMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[layerIndex]);

			for (int j = 0; j < prop.GetSrcObjectCount<FbxFileTexture>(); ++j)
			{
				auto pTexture = prop.GetSrcObject<FbxFileTexture>(j);
				if (pTexture)
				{
					//std::cout << pTexture->GetName() << " " << prop.GetName() << " " << pTexture->GetFileName() << std::endl;

					wchar_t path[256];
					size_t n;
					mbstowcs_s(&n, path, pTexture->GetFileName(), 256);

					auto pTexture = new Texture();
					if (FAILED(pTexture->LoadFromFile(path)))
					{
						SafeDelete(&pTexture);
						continue;
					}
					pTexturePtrs_->push_back(pTexture);
				}
			}
		}
	}

	return S_OK;
}

HRESULT Material::UpdateResources(Device* pDevice)
{
	auto pMaterial = NativePtr();

	//std::cout << pMaterial->GetName() << " " << pMaterial->GetClassId().GetName() << std::endl;

	for (auto pTexture : *pTexturePtrs_)
	{
		pTexture->UpdateResources(pDevice);
	}

	return S_OK;
}

UpdateSubresourceContext* Material::UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext)
{
	for (auto pTexture : *pTexturePtrs_)
	{
		pTexture->UpdateSubresource(pCommandList, pContext);
	}
	return pContext;
}

Material* Material::CreateReference()
{
	auto other = new Material(NativePtr());
	other->isReference_ = true;

	other->name_ = name_;
	other->pTexturePtrs_ = pTexturePtrs_;

	return other;
}
