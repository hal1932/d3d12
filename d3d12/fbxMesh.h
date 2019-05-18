#pragma once
#include "fbxObject.h"
#include "ConstantBufferView.h"
#include "fbxSkinCluster.h"
#include "NonCopyable.h"
#include <DirectXMath.h>
#include <Windows.h>

class Device;
class Resource;
class CommandList;
struct UpdateSubresourceContext;

namespace fbx
{
	class Material;
	class SkinCluster;

	__declspec(align(256))
	struct TransformConstant
	{
		DirectX::XMMATRIX World;
	};

	class Mesh : public TransformObject<FbxMesh>
	{
	public:
		struct Vertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT2 Texture0;
		};

	public:
		Mesh(FbxMesh* pMesh);
		~Mesh();

		Material* MaterialPtr() { return pMaterial_; }
		const Material* MaterialPtr() const { return pMaterial_; }

		Resource* VertexBuffer() { return pVertexBuffer_; }
		int VertexCount() { return *pVertexCount_; }

		Resource* IndexBuffer() { return pIndexBuffer_; }
		int IndexCount() { return *pIndexCount_; }

		ConstantBufferView<TransformConstant>* TransformBufferPtr() { return &transformCbv_; }

		HRESULT UpdateResources(Device* pDevice);
		UpdateSubresourceContext* UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext);

		HRESULT Setup();
		HRESULT LoadSkinClusters();
		size_t SkinClusterCount() { return skinClusterPtrs_.size(); }
		SkinCluster* SkinClusterPtr(size_t index) { return skinClusterPtrs_[index].get(); }

		Mesh* CreateReference();

		void SetupBuffers(ResourceViewHeap* pHeap)
		{
			transformCbv_.Setup(pHeap);
		}

		void SetTransform(const DirectX::XMMATRIX& t)
		{
			TransformConstant cb;
			cb.World = PoseMatrix() * t;
			transformCbv_.CopyBufferFrom(cb);
		}

	private:
		bool isReference_ = false;

		Resource* pVertexBuffer_ = nullptr;
		int* pVertexCount_ = nullptr;

		Resource* pIndexBuffer_ = nullptr;
		int* pIndexCount_ = nullptr;

		Material* pMaterial_ = nullptr;

		std::vector<std::unique_ptr<SkinCluster>> skinClusterPtrs_;

		ConstantBufferView<TransformConstant> transformCbv_;

		void UpdateVertexResources_(Device* pDevice);
		void UpdateIndexResources_(Device* pDevice);
	};

}// namespace fbx
