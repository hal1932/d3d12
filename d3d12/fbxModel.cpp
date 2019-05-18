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

Model::Model(FbxNode* pNode)
	: TransformObject(pNode)
{}


Model::~Model()
{
	SafeDeleteSequence(&meshPtrs_);
}


HRESULT Model::Setup()
{
	SafeDeleteSequence(&meshPtrs_);

	auto result = S_OK;

	auto& meshes = meshPtrs_;
	auto pRootNode = NativePtr();
	TraverseDepthFirst(pRootNode, FbxNodeAttribute::eMesh, [&result, &meshes](auto pNode)
	{
		auto pMesh = new Mesh(pNode->GetMesh());
		result = pMesh->Setup();
		if (FAILED(result))
		{
			return false;
		}
		pMesh->LoadSkinClusters();
		meshes.push_back(pMesh);
		return true;
	});

	return result;
}


HRESULT Model::UpdateResources(Device* pDevice)
{
	auto result = S_OK;
	for (auto& pMesh : meshPtrs_)
	{
		result = pMesh->UpdateResources(pDevice);
		if (FAILED(result))
		{
			return result;
		}
	}
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
	auto other = new Model(NativePtr());
	other->isReference_ = true;

	other->name_ = name_;

	other->meshPtrs_.resize(meshPtrs_.size());
	for (auto i = 0; i < meshPtrs_.size(); ++i)
	{
		other->meshPtrs_[i] = meshPtrs_[i]->CreateReference();
	}

	return other;
}
