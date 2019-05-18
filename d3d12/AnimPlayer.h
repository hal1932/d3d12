#pragma once
#include "fbxAnimStack.h"


enum AnimWrapMode
{
	Once,
	Loop,
	LoopRoundTrip,
};


class AnimPlayer
{
public:
	AnimPlayer(fbx::AnimStack* pAnimStack)
		: pAnimStack_(pAnimStack)
	{}
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
	fbx::AnimStack* pAnimStack_;
	float startFrame_;
	float endFrame_;
	float currentFrame_;
	AnimWrapMode wrapMode_ = AnimWrapMode::Once;
	float frameStep_ = 1.0f;
};

