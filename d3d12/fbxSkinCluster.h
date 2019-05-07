#pragma once
#include "fbxCommon.h"

namespace fbx
{
	class SkinCluster
	{
	public:
		SkinCluster(FbxSkin* pSkin) {
			const auto clusterCount = pSkin->GetClusterCount();
			for (auto i = 0; i < clusterCount; ++i) {
				auto pCluster = pSkin->GetCluster(i);
				auto pLinkedNode = pCluster->GetLink();
				if (pLinkedNode == nullptr) {
					continue;
				}

				ClusterItem item;
				item.pLinkedNode = pLinkedNode;
				pCluster->GetTransformLinkMatrix(item.InitialPose);

				const auto indexCount = pCluster->GetControlPointIndicesCount();
				const auto indices = pCluster->GetControlPointIndices();
				const auto weights = pCluster->GetControlPointWeights();

				items_.push_back(item);
				auto& itemWeights = items_[items_.size() - 1].Weights;

				for (auto j = 0; j < indexCount; ++j) {
					itemWeights.emplace_back(indices[j], weights[j]);
				}
			}
		}
		~SkinCluster() {}

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
