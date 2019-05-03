#pragma once
#include <lib.h>
#include "Model.h"
#include "ModelArray.h"
#include "ModelPass.h"

class Graphics;

const int cModelGridSize = 2;
const int cThreadCount = 3;

class GameScene
{
public:
	static const size_t cModelCount = cModelGridSize * cModelGridSize * cModelGridSize;

public:
	~GameScene();

	void Setup(Graphics& g);
	void Calc(Graphics& g);
	void Draw(Graphics& g, GpuStopwatch* pStopwatch);

private:
	UniqueArray<Model> models_;

	ResourceViewHeap cbSrUavHeap_;

	CameraConstant cameraConstant_;
	ConstantBufferView<CameraConstant> cameraCbv_;

	Camera camera_;
	ShaderManager shaders_;

	CommandListQueue commandLists_;
	TaskQueue taskQueue_;

	CpuStopwatchBatch cpuTimer_;
	uint frameIndex_;

	ModelPass modelPass_;
};

