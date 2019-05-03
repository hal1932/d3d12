#include "Model.h"
#include "CameraConstant.h"


void Model::LoadFromFile(const char* filepath)
{
	modelPtr_ = std::make_unique<fbx::Model>();
	modelPtr_->LoadFromFile(filepath);
}


void Model::UpdateResources(Device* pDevice)
{
	modelPtr_->UpdateResources(pDevice);
}


void Model::SetupAsReference(Model* pModel)
{
	modelPtr_ = std::unique_ptr<fbx::Model>(pModel->modelPtr_->CreateReference());
}


void Model::SetupBuffers(ResourceViewHeap* pHeap)
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


UpdateSubresourceContext* Model::UpdateSubresources(CommandList* pCmdList, UpdateSubresourceContext* pContext)
{
	return modelPtr_->UpdateSubresources(pCmdList, pContext);
}


void Model::SetShaderHash(ulonglong hash)
{
	modelPtr_->SetShaderHash(hash);
}


void Model::SetAnimCurrentFrame(size_t frame)
{
	for (auto i = 0; i < modelPtr_->MeshCount(); ++i)
	{
		modelPtr_->MeshPtr(i)->AnimStackPtr(0)->SetCurrentFrame(frame);
	}
}


void Model::SetTransform(const DirectX::XMMATRIX& world)
{
	for (auto i = 0; i < modelPtr_->MeshCount(); ++i)
	{
		auto m = modelPtr_->MeshPtr(i)->AnimStackPtr(0)->NextFrame(true) * world;
		modelPtr_->MeshPtr(i)->SetTransform(m);
	}
}


void Model::CreateDrawCommand(ID3D12GraphicsCommandList* pNativeList, ConstantBufferView<CameraConstant>* pCameraCbv)
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
