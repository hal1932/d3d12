#pragma once
#include "common.h"
#include <Windows.h>

class CpuStopwatch
{
public:
	CpuStopwatch()
	{
		QueryPerformanceFrequency(&frequency_);
	}

	void SetName(const tstring& name) { name_ = name; }
	const tstring& Name() { return name_; }

	void Start()
	{
		end_.QuadPart = 0ULL;
		QueryPerformanceCounter(&begin_);
	}

	void Stop()
	{
		QueryPerformanceCounter(&end_);
	}

	void Reset()
	{
		begin_.QuadPart = 0ULL;
		end_.QuadPart = 0ULL;
	}

	double ElaspedMilliseconds()
	{
		return (end_.QuadPart - begin_.QuadPart) * 1000.0 / frequency_.QuadPart;
	}

private:
	tstring name_;
	LARGE_INTEGER begin_;
	LARGE_INTEGER end_;
	LARGE_INTEGER frequency_;
};

