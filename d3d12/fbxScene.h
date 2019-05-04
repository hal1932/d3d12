#pragma once
#include <fbxsdk.h>
#include <Windows.h>
#include <memory>


namespace fbx
{
	class Model;
	class AnimStack;

	class Scene
	{
	public:
		~Scene();

		HRESULT LoadFromFile(const char* filePath);

		std::unique_ptr<Model> CreateModel();
		std::unique_ptr<AnimStack> CreateAnimStack();

	private:
		FbxScene* pScene_ = nullptr;
		FbxImporter* pSceneImporter_ = nullptr;
	};

}// namespace fbx
