#pragma once
#include <lib.h>
#include "RenderPass.h"


class Model;
struct CameraConstant;


class ModelPass final : public RenderPass
{
public:
	~ModelPass() {}

	void SetModels(UniqueArray<Model>* pModels, ResourceViewHeap* pModelResourceViewHeap) {
		pModels_ = pModels;
		pModelResourceViewHeap_ = pModelResourceViewHeap;
	}

	void SetCamera(ConstantBufferView<CameraConstant>* pCameraCbv) {
		pCameraCbv_ = pCameraCbv;
	}

	void SetTaskQueue(TaskQueue* pTaskQueue) {
		pTaskQueue_ = pTaskQueue;
	}

	void SetScreenSize(int width, int height) {
		viewport_ = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) };
		scissorRect_ = { 0, 0, width, height };
	}

	void SetRenderTargets(UINT renderTargetCount, Resource** pRenderTargetPtrs, Resource* pDepthStencil) {
		renderTargetCount_ = renderTargetCount;
		pRenderTargetPtrs_ = pRenderTargetPtrs;
		pDepthStencil_ = pDepthStencil;
	}

	virtual void SetupCommandList(Device* pDevice, CommandQueue* pCommandQueue);
	virtual void SetupGpuResources(Device* pDevice);
	virtual void SetupRenderPipeline(Device* pDevice);
	virtual void Calc();
	virtual void OpenDraw();
	virtual void Draw();
	virtual void CloseDraw(CommandQueue* pCommandQueue);


private:
	UniqueArray<Model>* pModels_;
	ResourceViewHeap* pModelResourceViewHeap_;
	ConstantBufferView<CameraConstant>* pCameraCbv_;

	TaskQueue* pTaskQueue_;

	UINT renderTargetCount_;
	Resource** pRenderTargetPtrs_;
	Resource* pDepthStencil_;

	ShaderManager shaders_;
	RootSignature rootSignature_;

	std::map<u64, ComPtr<ID3D12PipelineState>> pPipelineStateObjs_;

	CommandQueue* pRootCommandQueue_;
	CommandListQueue commandListQueue_;
	CommandList* pMainCommandList_;

	D3D12_VIEWPORT viewport_;
	D3D12_RECT scissorRect_;
};

