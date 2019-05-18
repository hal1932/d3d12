#pragma once
#include "fbxObject.h"

namespace fbx
{
	class SkinCluster : public Object<FbxSkin>
	{
	public:
		SkinCluster(FbxSkin* pSkin);
		~SkinCluster() {}

		HRESULT Setup();

	private:
		struct WeightItem
		{
			int VertexIndex;
			double Value;
			WeightItem(int vi, double v) : VertexIndex(vi), Value(v) {}
		};
		struct ClusterItem
		{
			FbxNode* pLinkedNode;
			FbxAMatrix InitialPose;
			std::vector<WeightItem> Weights;
		};

		std::vector<ClusterItem> items_;
	};

}// namespace fbx
