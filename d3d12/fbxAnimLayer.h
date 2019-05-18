#pragma once
#include "fbxObject.h"

namespace fbx
{
	class AnimCurveNode;

	class AnimLayer : public Object<FbxAnimLayer>
	{
	public:
		AnimLayer(FbxAnimLayer* pAnimLayer);
		~AnimLayer() = default;

		HRESULT Setup();

		//size_t AnimCurveCount() { return pAnimLayer_->GetMemberCount(); }
		//std::unique_ptr<AnimCurveNode> CreateAnimCurveNode(size_t index) {
		//	return std::make_unique<AnimCurveNode>(pAnimLayer_->GetMember<FbxAnimCurveNode>(static_cast<int>(index)));
		//}

	private:
		std::vector<std::unique_ptr<AnimCurveNode>> animCurveNodePtrs_;
	};

}// namespace fbx
