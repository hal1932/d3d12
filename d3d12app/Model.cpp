#include "Model.h"
#include "CameraConstant.h"


void Model::LoadFromFile(const char* filePath)
{
	pScene_ = std::make_unique<fbx::Scene>();
	pScene_->LoadFromFile(filePath);

	pModel_ = pScene_->CreateModel();
	pModel_->Setup();

	jointPtrs_ = pScene_->CreateJoints();
	for (auto& pJoint : jointPtrs_)
	{
		pJoint->Setup();
	}
}


void Model::LoadAnim(size_t index)
{
	pAnimStack_ = pScene_->CreateAnimStack(index);
	pAnimStack_->Setup();
}


void Model::UpdateResources(Device* pDevice)
{
	pModel_->UpdateResources(pDevice);
}


void Model::SetupAsReference(Model* pModel)
{
	pModel_ = std::unique_ptr<fbx::Model>(pModel->pModel_->CreateReference());
}


void Model::SetupBuffers(ResourceViewHeap* pHeap)
{
	for (auto i = 0; i < pModel_->MeshCount(); ++i)
	{
		pModel_->MeshPtr(i)->SetupBuffers(pHeap);
	}

	if (pModel_->MeshPtr(0)->MaterialPtr()->TextureCount() > 0)
	{
		const SrvDesc srvDesc = {
			D3D12_SRV_DIMENSION_TEXTURE2D,
			{ pModel_->MeshPtr(0)->MaterialPtr()->TexturePtr(0) } };
		pTextureSrv_ = pHeap->CreateShaderResourceView(srvDesc);
	}

	pResourceViewHeap_ = pHeap;
}


UpdateSubresourceContext* Model::UpdateSubresources(CommandList* pCmdList, UpdateSubresourceContext* pContext)
{
	return pModel_->UpdateSubresources(pCmdList, pContext);
}


void Model::SetShaderHash(u64 hash)
{
	pModel_->SetShaderHash(hash);
}


//void Model::SetAnimCurrentFrame(size_t frame)
//{
//	for (auto i = 0; i < modelPtr_->MeshCount(); ++i)
//	{
//		modelPtr_->MeshPtr(i)->AnimStackPtr(0)->SetCurrentFrame(frame);
//	}
//}


void Model::SetTransform(const DirectX::XMMATRIX& world)
{
	for (auto i = 0; i < pModel_->MeshCount(); ++i)
	{
		//auto m = modelPtr_->MeshPtr(i)->AnimStackPtr(0)->NextFrame(true) * world;
		pModel_->MeshPtr(i)->SetTransform(world);
	}
}


void Model::CreateDrawCommand(ID3D12GraphicsCommandList* pNativeList, ConstantBufferView<CameraConstant>* pCameraCbv)
{
	pNativeList->SetGraphicsRootDescriptorTable(1, pCameraCbv->GpuDescriptorHandle());

	if (pTextureSrv_ != nullptr)
	{
		pNativeList->SetGraphicsRootDescriptorTable(2, pTextureSrv_->GpuDescriptorHandle());
	}

	for (auto i = 0; i < pModel_->MeshCount(); ++i)
	{
		const auto pMesh = pModel_->MeshPtr(i);

		pNativeList->SetGraphicsRootDescriptorTable(0, pMesh->TransformBufferPtr()->GpuDescriptorHandle());

		auto vbView = pMesh->VertexBuffer()->GetVertexBufferView(sizeof(fbx::Mesh::Vertex));
		pNativeList->IASetVertexBuffers(0, 1, &vbView);

		auto ibView = pMesh->IndexBuffer()->GetIndexBufferView(DXGI_FORMAT_R16_UINT);
		pNativeList->IASetIndexBuffer(&ibView);

		pNativeList->DrawIndexedInstanced(pMesh->IndexCount(), 1, 0, 0, 0);
	}
}
