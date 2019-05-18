#include "fbxSkinCluster.h"
#include <iostream>

using namespace fbx;


SkinCluster::SkinCluster(FbxSkin* pSkin)
	: Object(pSkin)
{}

HRESULT SkinCluster::Setup()
{
	auto pSkin = NativePtr();

	const auto clusterCount = pSkin->GetClusterCount();
	for (auto i = 0; i < clusterCount; ++i)
	{
		auto pCluster = pSkin->GetCluster(i);
		auto pLinkedNode = pCluster->GetLink();
		if (pLinkedNode == nullptr)
		{
			continue;
		}

		ClusterItem item;
		item.pLinkedNode = pLinkedNode;
		pCluster->GetTransformLinkMatrix(item.InitialPose);

		std::cout << "skinCluster: " << pCluster->GetUniqueID() << " -> " << pLinkedNode->GetName() << " " << pLinkedNode->GetUniqueID() << std::endl;

		const auto indexCount = pCluster->GetControlPointIndicesCount();
		const auto indices = pCluster->GetControlPointIndices();
		const auto weights = pCluster->GetControlPointWeights();

		items_.push_back(item);
		auto& itemWeights = items_[items_.size() - 1].Weights;

		for (auto j = 0; j < indexCount; ++j)
		{
			itemWeights.emplace_back(indices[j], weights[j]);
		}
	}

	return S_OK;
}

