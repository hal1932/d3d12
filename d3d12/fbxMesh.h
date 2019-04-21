#pragma once
#include "fbxsdk.h"
#include "Transform.h"
#include "ConstantBuffer.h"
#include <DirectXMath.h>
#include <Windows.h>

__declspec(align(256))
struct TransformBuffer
{
	DirectX::XMMATRIX World;
};

class Device;
class Resource;
class CommandList;
class CommandQueue;

namespace fbx
{
	class Material;
	class AnimStack;

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

		HRESULT UpdateResources(FbxMesh* pMesh, FbxPose* pBindPose, Device* pDevice);
		HRESULT UpdateSubresources(CommandList* pCommandList, CommandQueue* pCommandQueue);

		Mesh* CreateReference();

		void LoadAnimStacks(FbxMesh* pMesh, FbxScene* pScene, FbxImporter* pSceneImporter);
		int AnimStackCount() { return animStackCount_; }
		AnimStack* AnimStackPtr(int index) { return pAnimStacks_[index]; }

		void SetupBuffers(ResourceViewHeap* pHeap)
		{
			transformCbv_.Setup(pHeap);
		}

		void SetTransform(const DirectX::XMMATRIX& t)
		{
			TransformBuffer buffer;
			buffer.World = initialPose_.Matrix() * t;
			transformCbv_.SetBuffer(buffer);
		}

		void SetRootDescriptorTable(ID3D12GraphicsCommandList* pList, int index)
		{
			pList->SetGraphicsRootDescriptorTable(index, transformCbv_.GpuDescriptorHandle());
		}

	private:
		bool isReference_ = false;

		Resource* pVertexBuffer_ = nullptr;
		int* pVertexCount_ = nullptr;

		Resource* pIndexBuffer_ = nullptr;
		int* pIndexCount_ = nullptr;

		Material* pMaterial_ = nullptr;
		Transform initialPose_;

		int animStackCount_;
		AnimStack** pAnimStacks_ = nullptr;

		ConstantBuffer<TransformBuffer> transformCbv_;

		void Setup_();
		void UpdateVertexResources_(FbxMesh* pMesh, Device* pDevice);
		void UpdateIndexResources_(FbxMesh* pMesh, Device* pDevice);
	};

}// namespace fbx
