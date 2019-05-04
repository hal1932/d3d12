#include "stdafx.h"
#include "fbxScene.h"
#include "common.h"
#include "fbxCommon.h"
#include "fbxModel.h"
#include <iostream>


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


std::unique_ptr<Model> Scene::CreateModel()
{
	return std::move(std::make_unique<Model>(pScene_));
}
