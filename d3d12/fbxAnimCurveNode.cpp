#include "fbxAnimCurveNode.h"
#include <iostream>

using namespace fbx;


AnimCurveNode::AnimCurveNode(FbxAnimCurveNode* pAnimCurveNode)
	: Object(pAnimCurveNode)
{}


HRESULT AnimCurveNode::Setup() {
	auto pAnimCurveNode = NativePtr();

	auto dest = pAnimCurveNode->GetDstProperty();
	auto pDestObj = dest.GetFbxObject();
	std::cout << "  node: " << pAnimCurveNode->GetName() << " -> " << pDestObj->GetName() << "." << dest.GetName() << std::endl;

	auto prop = pAnimCurveNode->GetFirstProperty();
	while (prop.IsValid()) {
		std::cout << prop.GetName() << std::endl;
		auto pAnimCurve = prop.GetSrcObject<FbxAnimCurve>();
		if (pAnimCurve != nullptr) {
			propCurveMap_[prop] = pAnimCurve->GetUniqueID();
		}
		prop = pAnimCurveNode->GetNextProperty(prop);
	}
	return S_OK;
}
