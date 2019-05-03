#pragma once
#include "common.h"
#include "Transform.h"
#include <fbxsdk.h>
#include <Windows.h>
#include <DirectXMath.h>
#include <vector>


class Device;
class Resource;
class CommandList;
class Texture;
struct UpdateSubresourceContext;

namespace fbx
{
	class Mesh;

	class Model
	{
	public:
		Model();
		~Model();

		Transform* TransformPtr() { return &transform_; }

		int MeshCount() { return static_cast<int>(meshPtrs_.size()); }
		int MeshCount() const { return static_cast<int>(meshPtrs_.size()); }

		const ulonglong ShaderHash() { return shaderHash_; }
		const ulonglong ShaderHash() const { return shaderHash_; }

		Mesh* MeshPtr(int index) { return meshPtrs_[index]; }
		const Mesh* MeshPtr(int index) const { return meshPtrs_[index]; }

		HRESULT LoadFromFile(const char* filepath);
		HRESULT UpdateResources(Device* pDevice);
		UpdateSubresourceContext* UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext);

		void SetShaderHash(ulonglong hash) { shaderHash_ = hash; }

		Model* CreateReference();

	private:
		bool isReference_ = false;

		tstring name_;
		FbxScene* pScene_ = nullptr;
		FbxImporter* pSceneImporter_ = nullptr;
		std::vector<Mesh*> meshPtrs_;

		Transform transform_;

		ulonglong shaderHash_;

		HRESULT UpdateResourcesRec_(fbxsdk::FbxNode* pNode, Device* pDevice);
		void UpdateMaterialResources_(fbxsdk::FbxGeometry* pMesh, Device* pDevice);
	};

}// namespace fbx
