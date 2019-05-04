#include "fbxModel.h"
#include "common.h"
#include "Device.h"
#include "Resource.h"
#include "Texture.h"
#include "fbxMesh.h"
#include "fbxCommon.h"
#include <iostream>
#include <vector>

using namespace fbx;
using namespace fbxsdk;

Model::Model(FbxScene* pScene)
	: pScene_(pScene)
{}


Model::~Model()
{
	SafeDeleteSequence(&meshPtrs_);
}


HRESULT Model::UpdateResources(Device* pDevice)
{
	SafeDeleteSequence(&meshPtrs_);

	auto pNode = pScene_->GetRootNode();
	return UpdateResourcesRec_(pNode, pDevice);
}


UpdateSubresourceContext* Model::UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext)
{
	for (auto pMesh : meshPtrs_)
	{
		pContext = pMesh->UpdateSubresources(pCommandList, pContext);
	}
	return pContext;
}


void Model::LoadAnimStacks(int meshIndex)
{
	//MeshPtr(meshIndex)->LoadAnimStacks(pScene_, pSceneImporter_);
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


HRESULT Model::UpdateResourcesRec_(FbxNode* pNode, Device* pDevice)
{
	if (!pNode)
	{
		return S_FALSE;
	}

	//std::cout << pNode->GetName() << " " << pNode->GetTypeName() << std::endl;

	auto pAttribute = pNode->GetNodeAttribute();
	if (pAttribute)
	{
		name_ = pAttribute->GetName();

		auto type = pAttribute->GetAttributeType();
		switch (type)
		{
			case FbxNodeAttribute::eMesh:
			{
				auto pMesh = new Mesh(pNode->GetMesh());
				pMesh->UpdateResources(pDevice);
				meshPtrs_.push_back(pMesh);
				break;
			}

			default:
				break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); ++i)
	{
		UpdateResourcesRec_(pNode->GetChild(i), pDevice);
	}

	return S_OK;
}
