#pragma once
#include "fbxCommon.h"
#include "fbxAnimCurveNode.h"

namespace fbx
{
	class AnimLayer
	{
	public:
		AnimLayer(FbxAnimLayer* pAnimLayer)
			: pAnimLayer_(pAnimLayer)
		{}
		~AnimLayer() {}

		size_t AnimCurveCount() { return pAnimLayer_->GetMemberCount(); }
		std::unique_ptr<AnimCurveNode> CreateAnimCurve(size_t index) {
			return std::make_unique<AnimCurveNode>(pAnimLayer_->GetMember<FbxAnimCurveNode>(static_cast<int>(index)));
		}

	private:
		FbxAnimLayer* pAnimLayer_;
	};

}// namespace fbx
