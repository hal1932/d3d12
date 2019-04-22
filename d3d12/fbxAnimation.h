#pragma once
#include "common.h"
#include "Transform.h"
#include "fbxCommon.h"
#include <fbxsdk.h>
#include <Windows.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>

namespace fbx
{
	class Animation
	{
	public:
		~Animation() 
		{
			SafeDestroy(&pScene_);
		}

		HRESULT LoadFromFile(const char* filepath)
		{
			auto pSceneImporter = FbxImporter::Create(GetManager(), "");

			auto result = pSceneImporter->Initialize(filepath, -1, GetManager()->GetIOSettings());
			if (!result)
			{
				SafeDestroy(&pSceneImporter);
				return S_FALSE;
			}

			SafeDestroy(&pScene_);
			pScene_ = FbxScene::Create(GetManager(), "");

			result = pSceneImporter->Import(pScene_);
			if (!result)
			{
				SafeDestroy(&pSceneImporter);
				return S_FALSE;
			}

			const auto animCount = pSceneImporter->GetAnimStackCount();
			for (auto i = 0; i < animCount; ++i)
			{
				auto pTakeInfo = pSceneImporter->GetTakeInfo(i);
				if (pTakeInfo != nullptr)
				{
					const auto start = pTakeInfo->mLocalTimeSpan.GetStart();
					const auto stop = pTakeInfo->mLocalTimeSpan.GetStop();
					std::cout << pTakeInfo->mName.Buffer() << std::endl;
					
					fbxsdk::FbxTime period;
					period.SetTime(0, 0, 0, 1, 0, pScene_->GetGlobalSettings().GetTimeMode());
					std::cout << start.Get() / period.Get() << " " << stop.Get() / period.Get() << std::endl;
				}
			}

			SafeDestroy(&pSceneImporter);
			return S_OK;
		}

	private:
		FbxScene* pScene_ = nullptr;
	};
}// namespace fbx
