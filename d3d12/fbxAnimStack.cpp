#include "fbxAnimStack.h"
#include "fbxAnimLayer.h"

using namespace fbx;


AnimStack::AnimStack(FbxAnimStack* pAnimStack)
	: Object(pAnimStack)
{}


LRESULT AnimStack::Setup() {
	auto pAnimStack = NativePtr();

	const auto layerCount = pAnimStack->GetMemberCount();

	auto result = S_OK;
	for (auto i = 0; i < layerCount; ++i) {
		auto pAnimLayer = std::make_unique<AnimLayer>(pAnimStack->GetMember<FbxAnimLayer>(i));
		result = pAnimLayer->Setup();
		if (FAILED(result)) {
			return result;
		}
		animLayerPtrs_.push_back(std::move(pAnimLayer));
	}

	return result;
}
