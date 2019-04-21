#pragma once
#include "CpuStopwatch.h"
#include "GpuStopwatch.h"

class FrameCounter
{
public:
	FrameCounter(CpuStopwatch* pCpuWatch, GpuStopwatch* pGpuWatch)
		: pCpuWatch_(pCpuWatch), pGpuWatch_(pGpuWatch)
	{}

	CpuStopwatch* CpuWatchPtr() { return pCpuWatch_; }
	GpuStopwatch* GpuWatchPtr() { return pGpuWatch_; }

	int FrameCount() { return frameCount_; }
	double CpuTime() { return cpuTime_; }
	double GpuTime() { return gpuTime_; }

	double AverageCpuTime() { return cpuTime_ / frameCount_; }
	double AverageGpuTime() { return gpuTime_ / frameCount_; }

	double CpuUtilization(int targetFrameCount) { return AverageCpuTime() * 100.0 / (1000.0 / targetFrameCount); }
	double GpuUtilization(int targetFrameCount) { return AverageGpuTime() * 100.0 / (1000.0 / targetFrameCount); }

	void NextFrame()
	{
		cpuTime_ += pCpuWatch_->ElaspedMilliseconds();
		gpuTime_ += pGpuWatch_->ElaspedMilliseconds();
		++frameCount_;
	}

	void Reset()
	{
		frameCount_ = 0;
		cpuTime_ = 0.0;
		gpuTime_ = 0.0;
	}

private:
	int frameCount_ = 0;
	double cpuTime_ = 0.0;
	double gpuTime_ = 0.0;

	CpuStopwatch* pCpuWatch_;
	GpuStopwatch* pGpuWatch_;
};

