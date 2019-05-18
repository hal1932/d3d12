#include "stdafx.h"
#include "fbxScene.h"
#include "common.h"
#include "fbxCommon.h"
#include "fbxModel.h"
#include "fbxAnimStack.h"
#include "fbxAnimCurve.h"
#include "fbxJoint.h"
#include "fbxMaterial.h"
#include <iostream>
#include <stack>
#include <queue>
#include <tuple>


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

	pSceneImporter_ = pSceneImporter;

	return S_OK;

}


std::unique_ptr<Model> Scene::LoadModel()
{
	return std::move(std::make_unique<Model>(pScene_->GetRootNode()));
}


std::unique_ptr<AnimStack> Scene::LoadAnimStack(size_t index)
{
	auto pAnimStack = pScene_->GetSrcObject<FbxAnimStack>(static_cast<int>(index));
	return std::move(std::make_unique<AnimStack>(pAnimStack));
}

std::unique_ptr<AnimCurve> Scene::LoadAnimCurve(size_t index)
{
	auto pAnimCurve = pScene_->GetSrcObject<FbxAnimCurve>(static_cast<int>(index));
	return std::move(std::make_unique<AnimCurve>(pAnimCurve));
}

std::vector<std::unique_ptr<Joint>> Scene::LoadJoints()
{
	std::vector<std::unique_ptr<Joint>> joints;

	auto pRootNode = pScene_->GetRootNode();
	TraverseDepthFirst(pRootNode, FbxNodeAttribute::eSkeleton, [&joints](auto pRootSkeletonNode)
	{
		auto queue = std::queue<std::tuple<FbxNode*, Joint*>>();
		queue.emplace(pRootSkeletonNode, nullptr);

		while (!queue.empty())
		{
			auto item = queue.front();
			queue.pop();

			auto pNode = std::get<0>(item);
			auto pParent = std::get<1>(item);

			auto pJoint = new Joint(pNode->GetSkeleton(), pParent);
			joints.emplace_back(pJoint);

			for (auto i = 0; i < pNode->GetChildCount(); ++i)
			{
				auto pChild = pNode->GetChild(i);
				if (pChild->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eSkeleton)
				{
					continue;
				}
				queue.emplace(pChild, pJoint);
			}
		}

		return false;
	});
	return std::move(joints);
}

std::vector<std::unique_ptr<Material>> Scene::LoadMaterials()
{
	std::vector<std::unique_ptr<Material>> materials;

	for (auto i = 0; i < pScene_->GetMaterialCount(); ++i)
	{
		auto pMaterial = new Material(pScene_->GetMaterial(i));
		materials.emplace_back(pMaterial);
	}

	return std::move(materials);
}

