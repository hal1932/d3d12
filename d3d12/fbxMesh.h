#pragma once
#include "fbxsdk.h"
#include "Transform.h"
#include "ConstantBufferView.h"
#include <DirectXMath.h>
#include <Windows.h>

class Device;
class Resource;
class CommandList;
struct UpdateSubresourceContext;

namespace fbx
{
	class Material;
	class AnimStack;

	__declspec(align(256))
	struct TransformConstant
	{
		DirectX::XMMATRIX World;
	};

	class Mesh
	{
	public:
		struct Vertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT2 Texture0;
		};

	public:
		Mesh();
		~Mesh();

		Material* MaterialPtr() { return pMaterial_; }
		const Material* MaterialPtr() const { return pMaterial_; }

		Resource* VertexBuffer() { return pVertexBuffer_; }
		int VertexCount() { return *pVertexCount_; }

		Resource* IndexBuffer() { return pIndexBuffer_; }
		int IndexCount() { return *pIndexCount_; }

		ConstantBufferView<TransformConstant>* TransformBufferPtr() { return &transformCbv_; }

		HRESULT UpdateResources(FbxMesh* pMesh, Device* pDevice);
		UpdateSubresourceContext* UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext);

		Mesh* CreateReference();

		void LoadAnimStacks(FbxMesh* pMesh, FbxScene* pScene, FbxImporter* pSceneImporter);
		size_t AnimStackCount() { return pAnimStacks_.size(); }
		AnimStack* AnimStackPtr(int index) { return pAnimStacks_[index].get(); }

		void SetupBuffers(ResourceViewHeap* pHeap)
		{
			transformCbv_.Setup(pHeap);
		}

		void SetTransform(const DirectX::XMMATRIX& t)
		{
			TransformConstant cb;
			cb.World = initialPose_.Matrix() * t;
			transformCbv_.CopyBufferFrom(cb);
		}

	private:
		bool isReference_ = false;

		Resource* pVertexBuffer_ = nullptr;
		int* pVertexCount_ = nullptr;

		Resource* pIndexBuffer_ = nullptr;
		int* pIndexCount_ = nullptr;

		Material* pMaterial_ = nullptr;
		Transform initialPose_;

		std::vector<std::unique_ptr<AnimStack>> pAnimStacks_;

		ConstantBufferView<TransformConstant> transformCbv_;

		void Setup_();
		void UpdateVertexResources_(FbxMesh* pMesh, Device* pDevice);
		void UpdateIndexResources_(FbxMesh* pMesh, Device* pDevice);
	};

}// namespace fbx
