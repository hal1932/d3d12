#include "fbxAnimCurve.h"
#include <iostream>

using namespace fbx;


AnimCurve::AnimCurve(FbxAnimCurve* pAnimCurve)
	: Object(pAnimCurve)
{}


HRESULT AnimCurve::Setup()
{
	auto pAnimCurve = NativePtr();
	std::cout << "      curve: " << pAnimCurve->GetUniqueID() << std::endl;
	return S_OK;
}