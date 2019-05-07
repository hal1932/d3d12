#pragma once
#include "fbxCommon.h"

namespace fbx
{
	class AnimCurve
	{
	public:
		AnimCurve(FbxAnimCurve* pAnimCurve) : pAnimCurve_(pAnimCurve) {}
		~AnimCurve() {}

	private:
		FbxAnimCurve* pAnimCurve_;
	};

} // namespace fbx
