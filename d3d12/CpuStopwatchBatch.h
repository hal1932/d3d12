#pragma once
#include "common.h"
#include "CpuStopwatch.h"
#include <map>
#include <cstdio>

class CpuStopwatchBatch
{
public:
	void Start(int id, const tstring& name)
	{
		CpuStopwatch sw;
		sw.SetName(name);
		watches_[id] = sw;

		{
			auto ite = iterations_.find(id);
			if (ite == iterations_.end())
			{
				iterations_[id] = 0;
			}
		}

		{
			auto ite = times_.find(id);
			if (ite == times_.end())
			{
				times_[id] = 0.0;
			}
		}

		watches_[id].Start();
	}

	void Stop(int id)
	{
		auto& sw = watches_[id];
		sw.Stop();
		times_[id] += sw.ElaspedMilliseconds();
		++iterations_[id];
	}

	void Reset()
	{
		watches_.clear();
		iterations_.clear();
		times_.clear();
	}

	int Iteration(int id)
	{
		return iterations_[id];
	}

	bool DumpAll(int checkId, int checkIteration)
	{
		if (iterations_[checkId] >= checkIteration)
		{
			printf("===[CPU]=====\n");
			for (auto& item : watches_)
			{
				auto id = item.first;
				auto time = times_[id] / iterations_[id];
				printf("%d %2.3f %s\n", id, time, item.second.Name().c_str());
			}
			return true;
		}
		return false;
	}

private:
	std::map<int, CpuStopwatch> watches_;
	std::map<int, int> iterations_;
	std::map<int, double> times_;
};

