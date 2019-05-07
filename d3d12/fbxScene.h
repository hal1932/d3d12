#pragma once
#include <fbxsdk.h>
#include <Windows.h>
#include <memory>
#include <functional>
#include "fbxAnimCurve.h"

namespace fbx
{
	class Model;
	class AnimStack;

	class Scene
	{
	public:
		~Scene();

		FbxScene* NativePtr() { return pScene_; }

		HRESULT LoadFromFile(const char* filePath);

		size_t AnimStackCount() { return pScene_->GetSrcObjectCount<FbxAnimStack>(); }
		AnimCurve* AnimCurvePtr(FbxUInt64 uniqueId) { return animCurvePtrs_[uniqueId].get(); }

		std::unique_ptr<Model> CreateModel();
		std::unique_ptr<AnimStack> CreateAnimStack(size_t index);

		void GetFbxNodeRecursive(FbxNodeAttribute::EType type, std::function<void(FbxNode*)> callback);

	private:
		FbxScene* pScene_ = nullptr;
		FbxImporter* pSceneImporter_ = nullptr;
		std::map<FbxUInt64, std::unique_ptr<AnimCurve>> animCurvePtrs_;
	};

}// namespace fbx
