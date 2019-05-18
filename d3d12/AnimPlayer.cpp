#include "stdafx.h"
#include "AnimPlayer.h"
#include <algorithm>


DirectX::XMMATRIX AnimPlayer::Play()
{
	const auto current = CurrentFrame();
	const auto matrix = pAnimCurve_->Evaluate(current);

	const auto nextFrame = current + FrameStep();

	switch (wrapMode_)
	{
		case AnimWrapMode::Once:
			if (nextFrame < EndFrame())
			{
				SetCurrentFrame(nextFrame);
			}
			break;

		case AnimWrapMode::Loop:
			SetCurrentFrame(nextFrame < EndFrame() ? nextFrame : StartFrame());
			break;

		case AnimWrapMode::LoopRoundTrip:
			if (StartFrame() <= nextFrame && nextFrame < EndFrame())
			{
				SetCurrentFrame(nextFrame);
			}
			else
			{
				SetFrameStep(FrameStep() * -1.0f);
			}
			break;
	}

	return matrix;
}
