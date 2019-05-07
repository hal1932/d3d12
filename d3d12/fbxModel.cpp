#include "fbxModel.h"
#include "common.h"
#include "Device.h"
#include "Resource.h"
#include "Texture.h"
#include "fbxMesh.h"
#include "fbxCommon.h"
#include "fbxScene.h"
#include <iostream>
#include <vector>

using namespace fbx;
using namespace fbxsdk;

Model::Model(Scene* pScene)
	: pScene_(pScene)
{}


Model::~Model()
{
	SafeDeleteSequence(&meshPtrs_);
}


HRESULT Model::UpdateResources(Device* pDevice)
{
	SafeDeleteSequence(&meshPtrs_);

	auto result = S_OK;

	auto& meshes = meshPtrs_;
	pScene_->GetFbxNodeRecursive(FbxNodeAttribute::eMesh, [&result, &meshes, pDevice](auto pNode)
	{
		auto pMesh = new Mesh(pNode->GetMesh());
		result = pMesh->UpdateResources(pDevice);
		if (SUCCEEDED(result))
		{
			meshes.push_back(pMesh);
		}
		pMesh->LoadSkinClusters();
	});

	return result;
}


UpdateSubresourceContext* Model::UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext)
{
	for (auto pMesh : meshPtrs_)
	{
		pContext = pMesh->UpdateSubresources(pCommandList, pContext);
	}
	return pContext;
}


Model* Model::CreateReference()
{
	auto other = new Model(pScene_);
	other->isReference_ = true;

	other->name_ = name_;

	other->meshPtrs_.resize(meshPtrs_.size());
	for (auto i = 0; i < meshPtrs_.size(); ++i)
	{
		other->meshPtrs_[i] = meshPtrs_[i]->CreateReference();
	}

	return other;
}
