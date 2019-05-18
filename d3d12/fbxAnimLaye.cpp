#include "fbxAnimLayer.h"
#include "fbxAnimCurveNode.h"
#include <iostream>

using namespace fbx;


AnimLayer::AnimLayer(FbxAnimLayer* pAnimLayer)
	: Object(pAnimLayer)
{}

HRESULT AnimLayer::Setup() {
	auto pAnimLayer = NativePtr();

	std::cout << "layer: " << pAnimLayer->GetName() << std::endl;

	const auto nodeCount = pAnimLayer->GetMemberCount();
	animCurveNodePtrs_.resize(nodeCount);

	auto result = S_OK;
	for (auto i = 0; i < nodeCount; ++i) {
		auto pAnimCurveNode = std::make_unique<AnimCurveNode>(pAnimLayer->GetMember<FbxAnimCurveNode>(i));
		result = pAnimCurveNode->Setup();
		if (FAILED(result)) {
			return result;
		}
		animCurveNodePtrs_.push_back(std::move(pAnimCurveNode));
	}

	return result;
}
