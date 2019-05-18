#pragma once
#include "fbxObject.h"
#include "fbxAnimCurveKey.h"

namespace fbx
{
	class AnimCurve : public Object<FbxAnimCurve>
	{
	public:
		AnimCurve(FbxAnimCurve* pAnimCurve);
		~AnimCurve() = default;

		HRESULT Setup();
	};

} // namespace fbx
