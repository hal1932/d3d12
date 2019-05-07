#pragma once
#include <fbxsdk.h>

namespace fbx
{
	fbxsdk::FbxManager* GetManager();

	void Setup();
	void Shutdown();

	inline void toFloat3(float pOut[3], const FbxDouble3& v)
	{
		for (auto i = 0; i < 3; ++i)
		{
			pOut[i] = static_cast<float>(v[i]);
		}
	}

	inline void toFloat3Radian(float pOut[3], const FbxDouble3& v)
	{
		for (auto i = 0; i < 3; ++i)
		{
			pOut[i] = static_cast<float>(v[i] * 3.14159265358979323846 / 180.0);
		}
	}
}// namespace fbx
