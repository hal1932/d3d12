#pragma once
#include "fbxObject.h"

namespace fbx
{
	class Joint final : public TransformObject<FbxSkeleton>
	{
	public:
		Joint(FbxSkeleton* pSkeleton, Joint* pParentJoint);
		~Joint() {}

		Joint* ParentPtr() { return pParent_; }
		const DirectX::XMMATRIX& GlobalPoseMatrix() { return globalPose_; }

		HRESULT Setup();
		void UpdateGlobalPoseMatrix();

	private:
		Joint* pParent_;
		DirectX::XMMATRIX globalPose_;
	};

}// namespace fbx
