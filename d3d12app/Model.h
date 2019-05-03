#pragma once
#include <lib.h>
#include <memory>


struct CameraConstant;


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

	void LoadFromFile(const char* filepath);
	void SetupAsReference(Model* pModel);

	void SetupBuffers(ResourceViewHeap* pHeap);

	void UpdateResources(Device* pDevice);
	UpdateSubresourceContext* UpdateSubresources(CommandList* pCmdList, UpdateSubresourceContext* pContext);

	void SetShaderHash(ulonglong hash);
	void SetAnimCurrentFrame(size_t frame);
	void SetTransform(const DirectX::XMMATRIX& world);

	void CreateDrawCommand(ID3D12GraphicsCommandList* pNativeList, ConstantBufferView<CameraConstant>* pCameraCbv);

private:
	std::unique_ptr<fbx::Model> modelPtr_;

	ResourceViewHeap* pResourceViewHeap_;
	Resource* pTextureSrv_;

	ulonglong shaderHash_;
};

