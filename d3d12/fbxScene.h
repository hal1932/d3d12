#pragma once
#include "fbxCommon.h"
#include "fbxJoint.h"
#include "NonCopyable.h"
#include <Windows.h>
#include <memory>
#include <functional>

namespace fbx
{
	class Model;
	class AnimStack;
	class AnimCurve;
	class Material;

	class Scene : private NonCopyable<Scene>
	{
	public:
		~Scene();

		FbxScene* NativePtr() { return pScene_; }

		HRESULT LoadFromFile(const char* filePath);

		size_t AnimStackCount() { return pScene_->GetSrcObjectCount<FbxAnimStack>(); }
		size_t AnimCurveCount() { return pScene_->GetSrcObjectCount<FbxAnimCurve>(); }

		std::unique_ptr<Model> LoadModel();
		std::unique_ptr<AnimStack> LoadAnimStack(size_t index);
		std::unique_ptr<AnimCurve> LoadAnimCurve(size_t index);
		std::vector<std::unique_ptr<Joint>> LoadJoints();
		std::vector<std::unique_ptr<Material>> LoadMaterials();

	private:
		FbxScene* pScene_ = nullptr;
		FbxImporter* pSceneImporter_ = nullptr;
	};

}// namespace fbx
