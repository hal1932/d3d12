#pragma once
#include "fbxCommon.h"

namespace fbx
{
	class AnimCurveNode
	{
	public:
		AnimCurveNode(FbxAnimCurveNode* pAnimCurveNode) {
			auto prop = pAnimCurveNode->GetFirstProperty();
			while (prop.IsValid()) {
				auto pAnimCurve = prop.GetSrcObject<FbxAnimCurve>();
				if (pAnimCurve == nullptr) {
					pAnimCurves_[prop] = pAnimCurve;
				}
				prop = pAnimCurveNode->GetNextProperty(prop);
			}
		}
		~AnimCurveNode() {}

	private:
		std::map<FbxProperty, FbxAnimCurve*> pAnimCurves_;
	};

} // namespace fbx
