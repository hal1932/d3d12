#pragma once
#include "fbxObject.h"

namespace fbx
{
	class AnimCurveNode : public Object<FbxAnimCurveNode>
	{
	public:
		AnimCurveNode(FbxAnimCurveNode* pAnimCurveNode);
		~AnimCurveNode() = default;

		HRESULT Setup();

	private:
		std::map<FbxProperty, u64> propCurveMap_;
	};

} // namespace fbx
