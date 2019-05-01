#pragma once
#include <lib.h>
#include "RenderPass.h"

class ModelPass final : public RenderPass
{
public:
	~ModelPass();

	virtual void Setup(Graphics& g);
	virtual void Calc();
	virtual void Draw(Graphics& g, GpuStopwatch* pStopwatch);

private:
	ComPtr<ID3D12RootSignature> pRootSignature_;
	std::map<tstring, ComPtr<ID3D12PipelineState>> pPipelineStates_;

	D3D12_VIEWPORT viewport_;
	D3D12_RECT scissorRect_;

};

