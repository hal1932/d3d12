#pragma once

class AnimCurve
{
public:
	virtual ~AnimCurve() {}

	float Length() { return EndFrame() - StartFrame(); }

	virtual float StartFrame() = 0;
	virtual float EndFrame() = 0;

	virtual DirectX::XMMATRIX Evaluate(float frame) = 0;
};

