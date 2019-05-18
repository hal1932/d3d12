#pragma once
#include <lib.h>
#include <memory>


struct CameraConstant;


class Model
{
public:
	int BufferCount() { return 2; }

	Transform* TransformPtr() { return pModel_.get(); }

	fbx::Model& FbxModel() { return *pModel_; }
	const fbx::Model& FbxModel() const { return *pModel_; }

	u64 ShaderHash() { return pModel_->ShaderHash(); }
	u64 ShaderHash() const { return pModel_->ShaderHash(); }

	int MeshCount() { return pModel_->MeshCount(); }
	int MeshCount() const { return pModel_->MeshCount(); }

	int ResourceBufferCount() { return 2 * MeshCount(); } // 1CB(SRT) + 1SR(Color) (/Mesh)
	int ResourceBufferCount() const { return 2 * MeshCount(); }

	void LoadFromFile(const char* filePath);
	void LoadAnim(size_t index);
	//void LoadAnimStacks(int meshIndex) { return pModel_->LoadAnimStacks(meshIndex); }
	void SetupAsReference(Model* pModel);

	void SetupBuffers(ResourceViewHeap* pHeap);

	void UpdateResources(Device* pDevice);
	UpdateSubresourceContext* UpdateSubresources(CommandList* pCmdList, UpdateSubresourceContext* pContext);

	void SetShaderHash(u64 hash);
	//void SetAnimCurrentFrame(size_t frame);
	void SetTransform(const DirectX::XMMATRIX& world);

	void CreateDrawCommand(ID3D12GraphicsCommandList* pNativeList, ConstantBufferView<CameraConstant>* pCameraCbv);

private:
	std::unique_ptr<fbx::Scene> pScene_;
	std::unique_ptr<fbx::Model> pModel_;
	std::unique_ptr<fbx::AnimStack> pAnimStack_;
	std::vector<std::unique_ptr<fbx::Joint>> jointPtrs_;

	ResourceViewHeap* pResourceViewHeap_;
	Resource* pTextureSrv_ = nullptr;

	u64 shaderHash_;
};

