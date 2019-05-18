#pragma once
#include "fbxObject.h"
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
	class Scene;
	class Mesh;

	class Model : public TransformObject<FbxNode>
	{
	public:
		Model(FbxNode* pNode);
		~Model();

		int MeshCount() { return static_cast<int>(meshPtrs_.size()); }
		int MeshCount() const { return static_cast<int>(meshPtrs_.size()); }

		const u64 ShaderHash() { return shaderHash_; }
		const u64 ShaderHash() const { return shaderHash_; }

		Mesh* MeshPtr(int index) { return meshPtrs_[index]; }
		const Mesh* MeshPtr(int index) const { return meshPtrs_[index]; }

		HRESULT LoadFromFile(const char* filepath);
		HRESULT Setup();
		HRESULT UpdateResources(Device* pDevice);
		UpdateSubresourceContext* UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext);

		void SetShaderHash(u64 hash) { shaderHash_ = hash; }

		Model* CreateReference();

	private:
		bool isReference_ = false;

		tstring name_;
		std::vector<Mesh*> meshPtrs_;

		u64 shaderHash_;
	};

}// namespace fbx
