#pragma once
#include "common.h"
#include "fbxCommon.h"
#include "fbxAnimLayer.h"
#include "fbxScene.h"
#include <Windows.h>
#include <DirectXMath.h>

namespace fbx
{
	class AnimStack
	{
	public:
		AnimStack(Scene* pScene, size_t index) {
			pAnimStack_ = pScene->NativePtr()->GetSrcObject<FbxAnimStack>(static_cast<int>(index));
		}
		~AnimStack() = default;

		size_t AnimLayerCount() { return pAnimStack_->GetMemberCount(); }
		std::unique_ptr<AnimLayer> LoadAnimLayer(size_t index) {
			return std::make_unique<AnimLayer>(pAnimStack_->GetMember<FbxAnimLayer>(static_cast<int>(index)));
		}

	private:
		FbxAnimStack* pAnimStack_;
	};
}// namespace fbx
