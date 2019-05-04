#pragma once
#include "AnimCurve.h"


enum AnimWrapMode
{
	Once,
	Loop,
	LoopRoundTrip,
};


class AnimPlayer
{
public:
	AnimPlayer(AnimCurve* pAnimCurve)
		: AnimPlayer(pAnimCurve, pAnimCurve->StartFrame(), pAnimCurve->EndFrame())
	{}
	AnimPlayer(AnimCurve* pAnimCurve, float startFrame, float endFrame);
	~AnimPlayer() = default;

	float StartFrame() { return startFrame_; }
	float EndFrame() { return endFrame_; }
	float CurrentFrame() { return currentFrame_; }
	AnimWrapMode WrapMode() { return wrapMode_; }
	float FrameStep() { return frameStep_; }

	void SetCurrentFrame(float currentFrame) { currentFrame_ = currentFrame; }
	void SetWrapMode(AnimWrapMode wrapMode) { wrapMode_ = wrapMode; }
	void SetFrameStep(float frameStep) { frameStep_ = frameStep; }

	DirectX::XMMATRIX Play();

private:
	AnimCurve* pAnimCurve_;
	float startFrame_;
	float endFrame_;
	float currentFrame_;
	AnimWrapMode wrapMode_ = AnimWrapMode::Once;
	float frameStep_ = 1.0f;
};

