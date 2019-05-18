#pragma once
#include "fbxObject.h"

namespace fbx
{
	class AnimLayer;

	class AnimStack : public Object<FbxAnimStack>
	{
	public:
		AnimStack(FbxAnimStack* pAnimStack);
		~AnimStack() = default;

		LRESULT Setup();

		size_t AnimLayerCount() { return animLayerPtrs_.size(); }
		AnimLayer* AnimLayerPtr(size_t index) { return animLayerPtrs_[index].get(); }

	private:
		std::vector<std::unique_ptr<AnimLayer>> animLayerPtrs_;
	};
}// namespace fbx
