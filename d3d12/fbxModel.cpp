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

Model::Model() {}

Model::~Model()
{
	SafeDeleteSequence(&meshPtrs_);
	SafeDestroy(&pSceneImporter_);

	if (!isReference_)
	{
		SafeDestroy(&pScene_);
	}
}


HRESULT Model::LoadFromFile(const char* filepath)
{
	auto pSceneImporter = FbxImporter::Create(GetManager(), "");

	auto result = pSceneImporter->Initialize(filepath, -1, GetManager()->GetIOSettings());
	if (!result)
	{
		auto error = pSceneImporter->GetStatus().GetErrorString();
		std::cerr << error << std::endl;

		SafeDestroy(&pSceneImporter);
		return S_FALSE;
	}

	SafeDestroy(&pScene_);
	pScene_ = FbxScene::Create(GetManager(), "");

	result = pSceneImporter->Import(pScene_);
	if (!result)
	{
		auto error = pSceneImporter->GetStatus().GetErrorString();
		std::cerr << error << std::endl;

		SafeDestroy(&pSceneImporter);
		return S_FALSE;
	}

	pSceneImporter_ = pSceneImporter;

	return S_OK;
}

HRESULT Model::UpdateResources(Device* pDevice)
{
	SafeDeleteSequence(&meshPtrs_);

	auto pNode = pScene_->GetRootNode();
	return UpdateResourcesRec_(pNode, pDevice);
}

HRESULT Model::UpdateSubresources(CommandList* pCommandList, CommandQueue* pCommandQueue)
{
	HRESULT result;

	for (auto pMesh : meshPtrs_)
	{
		result |= pMesh->UpdateSubresources(pCommandList, pCommandQueue);
	}

	return result;
}

Model* Model::CreateReference()
{
	auto other = new Model();
	other->isReference_ = true;

	other->name_ = name_;
	other->pScene_ = pScene_;

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

	std::cout << pNode->GetName() << " " << pNode->GetTypeName() << std::endl;

	auto pAttribute = pNode->GetNodeAttribute();
	if (pAttribute)
	{
		name_ = pAttribute->GetName();

		auto type = pAttribute->GetAttributeType();
		switch (type)
		{
			case FbxNodeAttribute::eMesh:
			{
				auto pMesh = new Mesh();
				pMesh->UpdateResources(pNode->GetMesh(), pScene_->GetPose(0), pDevice);
				meshPtrs_.push_back(pMesh);

				pMesh->LoadAnimStacks(pNode->GetMesh(), pScene_, pSceneImporter_);

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
