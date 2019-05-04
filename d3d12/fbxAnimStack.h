#pragma once
#include "common.h"
#include "fbxCommon.h"
#include "AnimCurve.h"
#include <Windows.h>
#include <DirectXMath.h>

namespace fbx
{
	class AnimStack : public AnimCurve
	{
	public:
		AnimStack(FbxMesh* pMesh, fbxsdk::FbxTakeInfo* pTakeInfo, fbxsdk::FbxTime::EMode mode)
		{
			fbxsdk::FbxTime period;
			period.SetTime(0, 0, 0, 1, 0, mode);

			const auto start = pTakeInfo->mLocalTimeSpan.GetStart();
			const auto stop = pTakeInfo->mLocalTimeSpan.GetStop();

			start_ = (int)(start.Get() / period.Get());
			stop_ = (int)(stop.Get() / period.Get());

			const auto count = stop_ - start_ + 1;
			matrices_ = std::make_unique<DirectX::XMMATRIX[]>(count);

			const auto pNode = pMesh->GetNode();
			for (auto i = start_; i <= stop_; ++i)
			{
				const auto& m = pNode->EvaluateGlobalTransform(period * i);
				new (&matrices_[i]) DirectX::XMMATRIX(
					(float)m.Get(0, 0), (float)m.Get(0, 1), (float)m.Get(0, 2), (float)m.Get(0, 3),
					(float)m.Get(1, 0), (float)m.Get(1, 1), (float)m.Get(1, 2), (float)m.Get(1, 3),
					(float)m.Get(2, 0), (float)m.Get(2, 1), (float)m.Get(2, 2), (float)m.Get(2, 3),
					(float)m.Get(3, 0), (float)m.Get(3, 1), (float)m.Get(3, 2), (float)m.Get(3, 3));
			}
		}

		~AnimStack() {}

		//const DirectX::XMMATRIX& Matrix(int frame) { return matrices_[frame]; }
		virtual DirectX::XMMATRIX Evaluate(float frame) { return matrices_[static_cast<size_t>(frame)]; }
		//int FrameCount() { return stop_ - start_ + 1; }
		//int StartFrame() { return start_; }
		//int StopFrame() { return stop_; }
		virtual float StartFrame() { return static_cast<float>(start_); }
		virtual float EndFrame() { return static_cast<float>(stop_); }

		//void SetStartFrame(int start) { start_ = start; }
		//void SetStopFrame(int stop) { stop_ = stop; }
		//void SetCurrentFrame(int current) { current_ = current; }

		//const DirectX::XMMATRIX& NextFrame(bool loop)
		//{
		//	const auto& m = matrices_[current_];
		//	if (++current_ >= FrameCount())
		//	{
		//		current_ = loop ? 0 : current_ - 1;
		//	}
		//	return m;
		//}

	private:
		int start_;
		int stop_;
		std::unique_ptr<DirectX::XMMATRIX[]> matrices_;

		int current_ = 0;
	};
}// namespace fbx
