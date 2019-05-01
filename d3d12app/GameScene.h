#pragma once
#include <array>
#include <lib.h>
#include "Model.h"

class Graphics;

const int cModelGridSize = 1;
const int cThreadCount = 3;

class GameScene
{
public:
	~GameScene();

	void Setup(Graphics& g);
	void Calc(Graphics& g);
	void Draw(Graphics& g, GpuStopwatch* pStopwatch);

private:
	RootSignature rootSignature_;

	std::array<std::unique_ptr<Model>, cModelGridSize * cModelGridSize * cModelGridSize> modelPtrs_;
	std::map<tstring, ComPtr<ID3D12PipelineState>> pPipelineStates_;

	ResourceViewHeap cbSrUavHeap_;

	CameraConstant cameraConstant_;
	ConstantBufferView<CameraConstant> cameraCbv_;

	Camera camera_;
	ShaderManager shaders_;

	D3D12_VIEWPORT viewport_;
	D3D12_RECT scissorRect_;

	CommandListQueue commandLists_;
	TaskQueue taskQueue_;

	CpuStopwatchBatch cpuTimer_;
	uint frameIndex_;

	void CreateModelCommand(Graphics& g);
};

