#pragma once
#include <lib.h>
#include <memory>

__declspec(align(256))
struct CameraConstant
{
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
};

class Model
{
public:
	int BufferCount() { return 2; }

	Transform* TransformPtr() { return modelPtr_->TransformPtr(); }

	fbx::Model& FbxModel() { return *modelPtr_; }
	const fbx::Model& FbxModel() const { return *modelPtr_; }

	ulonglong ShaderHash() { return modelPtr_->ShaderHash(); }
	ulonglong ShaderHash() const { return modelPtr_->ShaderHash(); }

	int MeshCount() { return modelPtr_->MeshCount(); }
	int MeshCount() const { return modelPtr_->MeshCount(); }

	int ResourceBufferCount() { return 2 * MeshCount(); } // 1CB(SRT) + 1SR(Color) (/Mesh)
	int ResourceBufferCount() const { return 2 * MeshCount(); }

	void LoadFromFile(const char* filepath)
	{
		modelPtr_ = std::make_unique<fbx::Model>();
		modelPtr_->LoadFromFile(filepath);
	}

	void UpdateResources(Device* pDevice)
	{
		modelPtr_->UpdateResources(pDevice);
	}

	void SetupAsReference(Model* pModel)
	{
		modelPtr_ = std::unique_ptr<fbx::Model>(pModel->modelPtr_->CreateReference());
	}
	
	void SetupBuffers(ResourceViewHeap* pHeap)
	{
		for (auto i = 0; i < modelPtr_->MeshCount(); ++i)
		{
			modelPtr_->MeshPtr(i)->SetupBuffers(pHeap);
		}

		if (modelPtr_->MeshPtr(0)->MaterialPtr()->TexturePtr() != nullptr)
		{
			const SrvDesc srvDesc = {
				D3D12_SRV_DIMENSION_TEXTURE2D,
				{ modelPtr_->MeshPtr(0)->MaterialPtr()->TexturePtr() } };
			pTextureSrv_ = pHeap->CreateShaderResourceView(srvDesc);
		}

		pResourceViewHeap_ = pHeap;
	}

	UpdateSubresourceContext* UpdateSubresources(CommandList* pCmdList, UpdateSubresourceContext* pContext)
	{
		return modelPtr_->UpdateSubresources(pCmdList, pContext);
	}

	void SetShaderHash(ulonglong hash)
	{
		modelPtr_->SetShaderHash(hash);
	}

	void SetAnimCurrentFrame(size_t frame)
	{
		for (auto i = 0; i < modelPtr_->MeshCount(); ++i)
		{
			modelPtr_->MeshPtr(i)->AnimStackPtr(0)->SetCurrentFrame(frame);
		}
	}

	void SetTransform(const DirectX::XMMATRIX& world)
	{
		for (auto i = 0; i < modelPtr_->MeshCount(); ++i)
		{
			auto m = modelPtr_->MeshPtr(i)->AnimStackPtr(0)->NextFrame(true) * world;
			modelPtr_->MeshPtr(i)->SetTransform(m);
		}
	}

	void CreateDrawCommand(ID3D12GraphicsCommandList* pNativeList, ConstantBufferView<CameraConstant>* pCameraCbv)
	{
		pNativeList->SetGraphicsRootDescriptorTable(1, pCameraCbv->GpuDescriptorHandle());

		if (pTextureSrv_ != nullptr)
		{
			pNativeList->SetGraphicsRootDescriptorTable(2, pTextureSrv_->GpuDescriptorHandle());
		}

		for (auto i = 0; i < modelPtr_->MeshCount(); ++i)
		{
			const auto pMesh = modelPtr_->MeshPtr(i);

			pNativeList->SetGraphicsRootDescriptorTable(0, pMesh->TransformBufferPtr()->GpuDescriptorHandle());

			auto vbView = pMesh->VertexBuffer()->GetVertexBufferView(sizeof(fbx::Mesh::Vertex));
			pNativeList->IASetVertexBuffers(0, 1, &vbView);

			auto ibView = pMesh->IndexBuffer()->GetIndexBufferView(DXGI_FORMAT_R16_UINT);
			pNativeList->IASetIndexBuffer(&ibView);

			pNativeList->DrawIndexedInstanced(pMesh->IndexCount(), 1, 0, 0, 0);
		}
	}

private:
	std::unique_ptr<fbx::Model> modelPtr_;

	ResourceViewHeap* pResourceViewHeap_;
	Resource* pTextureSrv_;

	ulonglong shaderHash_;
};

