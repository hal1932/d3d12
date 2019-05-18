#include "stdafx.h"
#include "fbxJoint.h"

using namespace fbx;
using namespace DirectX;


Joint::Joint(FbxSkeleton* pSkeleton, Joint* pParentJoint)
	: TransformObject(pSkeleton),
	pParent_(pParentJoint)
{}


HRESULT Joint::Setup()
{
	const auto pNode = NativePtr()->GetNode();
	
	const auto& s = pNode->LclScaling.Get();
	const auto& r = pNode->LclRotation.Get();
	const auto& t = pNode->LclTranslation.Get();

	SetScaling(static_cast<float>(s[0]), static_cast<float>(s[1]), static_cast<float>(s[2]));
	SetRotation(static_cast<float>(r[0]), static_cast<float>(r[1]), static_cast<float>(r[2]));
	SetTranslation(static_cast<float>(t[0]), static_cast<float>(t[1]), static_cast<float>(t[2]));

	UpdateMatrix();

	return S_OK;
}

void Joint::UpdateGlobalPoseMatrix()
{
	if (pParent_ != nullptr)
	{
		globalPose_ = pParent_->GlobalPoseMatrix() * PoseMatrix();
	}
	else
	{
		globalPose_ = PoseMatrix();
	}
}
