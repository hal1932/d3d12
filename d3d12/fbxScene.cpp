#include "stdafx.h"
#include "fbxScene.h"
#include "common.h"
#include "fbxCommon.h"
#include "fbxModel.h"
#include "fbxAnimStack.h"
#include <iostream>
#include <stack>


using namespace fbx;


Scene::~Scene()
{
	SafeDestroy(&pSceneImporter_);
	SafeDestroy(&pScene_);
}


HRESULT Scene::LoadFromFile(const char* filePath)
{
	auto pSceneImporter = FbxImporter::Create(GetManager(), "");

	auto result = pSceneImporter->Initialize(filePath, -1, GetManager()->GetIOSettings());
	if (!result)
	{
		auto error = pSceneImporter->GetStatus().GetErrorString();
		std::cerr << error << std::endl;

		SafeDestroy(&pSceneImporter);
		return S_FALSE;
	}

	SafeDestroy(&pScene_);
	pScene_ = FbxScene::Create(GetManager(), "");
	if (pScene_ == nullptr)
	{
		auto error = pSceneImporter->GetStatus().GetErrorString();
		std::cerr << error << std::endl;

		SafeDestroy(&pSceneImporter);
		return S_FALSE;
	}

	result = pSceneImporter->Import(pScene_);
	if (!result)
	{
		auto error = pSceneImporter->GetStatus().GetErrorString();
		std::cerr << error << std::endl;

		SafeDestroy(&pSceneImporter);
		return S_FALSE;
	}

	const auto animCurveCount = pScene_->GetSrcObjectCount<FbxAnimCurve>();
	for (auto i = 0; i < animCurveCount; ++i)
	{
		auto pAnimCurve = pScene_->GetSrcObject<FbxAnimCurve>(i);
		animCurvePtrs_[pAnimCurve->GetUniqueID()] = std::make_unique<AnimCurve>(pAnimCurve);
	}

	pSceneImporter_ = pSceneImporter;

	return S_OK;

}


void Scene::GetFbxNodeRecursive(FbxNodeAttribute::EType type, std::function<void(FbxNode*)> callback)
{
	auto stack = std::stack<FbxNode*>();
	stack.push(pScene_->GetRootNode());

	while (!stack.empty())
	{
		auto pNode = stack.top();
		stack.pop();

		const auto pAttribute = pNode->GetNodeAttribute();
		if (pAttribute != nullptr)
		{
			const auto attrType = pAttribute->GetAttributeType();
			if (attrType == type)
			{
				callback(pNode);
			}
		}

		for (auto i = 0; i < pNode->GetChildCount(); ++i)
		{
			stack.push(pNode->GetChild(i));
		}
	}
}


std::unique_ptr<Model> Scene::CreateModel()
{
	return std::move(std::make_unique<Model>(this));
}


std::unique_ptr<AnimStack> Scene::CreateAnimStack(size_t index)
{
	return std::move(std::make_unique<AnimStack>(this, index));
}
