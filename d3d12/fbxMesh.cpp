#include "fbxMesh.h"
#include "common.h"
#include "Device.h"
#include "Resource.h"
#include "fbxMaterial.h"
#include "fbxCommon.h"
#include "fbxAnimStack.h"
#include <vector>
#include <iostream>

using namespace fbx;
using namespace fbxsdk;

Mesh::Mesh(FbxMesh* pMesh)
	: pMesh_(pMesh)
{}

Mesh::~Mesh()
{
	if (!isReference_)
	{
		SafeDelete(&pVertexCount_);
		SafeDelete(&pIndexCount_);

		SafeDelete(&pIndexBuffer_);
		SafeDelete(&pVertexBuffer_);
	}

	SafeDelete(&pMaterial_);
}

HRESULT Mesh::UpdateResources(Device* pDevice)
{
	Setup_();

	UpdateVertexResources_(pDevice);
	UpdateIndexResources_(pDevice);

	{
		auto pNode = pMesh_->GetNode();

		float v[3];
		toFloat3(v, pNode->LclScaling.Get());
		initialPose_.SetScaling(v[0], v[1], v[2]);

		toFloat3Radian(v, pNode->LclRotation.Get());
		initialPose_.SetRotation(v[0], v[1], v[2]);

		toFloat3(v, pNode->LclTranslation.Get());
		initialPose_.SetTranslation(v[0], v[1], v[2]);

		initialPose_.UpdateMatrix();
	}

	SafeDelete(&pMaterial_);
	pMaterial_ = new Material();

	return pMaterial_->UpdateResources(pMesh_, pDevice);
}

UpdateSubresourceContext* Mesh::UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext)
{
	return pMaterial_->UpdateSubresources(pCommandList, pContext);
}

HRESULT Mesh::LoadSkinClusters()
{
	for (auto i = 0; i < pMesh_->GetDeformerCount(FbxDeformer::EDeformerType::eSkin); ++i)
	{
		auto pSkin = static_cast<FbxSkin*>(pMesh_->GetDeformer(i, FbxDeformer::EDeformerType::eSkin));
		skinClusterPtrs_.push_back(std::make_unique<SkinCluster>(pSkin));
	}
	return S_OK;
}

Mesh* Mesh::CreateReference()
{
	auto other = new Mesh(pMesh_);
	other->isReference_ = true;

	other->pVertexBuffer_ = pVertexBuffer_;
	other->pVertexCount_ = pVertexCount_;
	
	other->pIndexBuffer_ = pIndexBuffer_;
	other->pIndexCount_ = pIndexCount_;

	other->pMaterial_ = pMaterial_->CreateReference();
	other->initialPose_ = initialPose_;

	return other;
}

void Mesh::Setup_()
{
	SafeDelete(&pVertexCount_);
	pVertexCount_ = new int();

	SafeDelete(&pIndexCount_);
	pIndexCount_ = new int();
}

void Mesh::UpdateVertexResources_(Device* pDevice)
{
	SafeDelete(&pVertexBuffer_);
	pVertexBuffer_ = new Resource();

	const auto controlPointCount = pMesh_->GetControlPointsCount();
	pVertexBuffer_->CreateVertexBuffer(pDevice, sizeof(Vertex) * controlPointCount);

	{
		const auto pData = static_cast<Vertex*>(pVertexBuffer_->Map(0));

		// 位置
		{
			const auto controlPoints = pMesh_->GetControlPoints();

			for (int i = 0; i < controlPointCount; ++i)
			{
				const auto& point = controlPoints[i];
				pData[i].Position.x = static_cast<float>(point.mData[0]);
				pData[i].Position.y = static_cast<float>(point.mData[1]);
				pData[i].Position.z = static_cast<float>(point.mData[2]);
			}
		}

		// GetDirectArray() するとシーン破棄時にアクセス違反で落ちるので
		// 以下 GetPolygonVertex 系の I/F で統一する

		// 法線
		{
			auto tmpNormalArrays = new std::vector<FbxVector4>[controlPointCount];

			for (int i = 0; i < pMesh_->GetPolygonCount(); ++i)
			{
				for (int j = 0; j < pMesh_->GetPolygonSize(i); ++j)
				{
					const auto dataIndex = pMesh_->GetPolygonVertex(i, j);

					FbxVector4 normal;
					if (pMesh_->GetPolygonVertexNormal(i, j, normal))
					{
						tmpNormalArrays[dataIndex].push_back(normal);
					}
				}
			}

			// 同じ頂点IDに複数の法線がはいってたら、とりあえず強制ソフトエッジ化
			for (int i = 0; i < controlPointCount; ++i)
			{
				if (tmpNormalArrays[i].size() > 1)
				{
					DirectX::XMFLOAT3 n = {};
					for (auto& normal : tmpNormalArrays[i])
					{
						n.x += static_cast<float>(normal.mData[0]);
						n.y += static_cast<float>(normal.mData[1]);
						n.z += static_cast<float>(normal.mData[2]);
					}

					const auto denomi = 1.0f / tmpNormalArrays[i].size();
					n.x *= denomi;
					n.y *= denomi;
					n.z *= denomi;

					XMFloat3Normalize(&pData[i].Normal, &n);
				}
				else
				{
					const auto& normal = tmpNormalArrays[i][0];
					pData[i].Normal.x = static_cast<float>(normal[0]);
					pData[i].Normal.y = static_cast<float>(normal[1]);
					pData[i].Normal.z = static_cast<float>(normal[2]);
				}
			}

			SafeDeleteArray(&tmpNormalArrays);
		}

		// UV
		{
			for (int i = 0; i < pMesh_->GetPolygonCount(); ++i)
			{
				for (int j = 0; j < pMesh_->GetPolygonSize(i); ++j)
				{
					FbxVector2 uv;
					bool unmapped;
					if (pMesh_->GetPolygonVertexUV(i, j, "map1", uv, unmapped))
					{
						const auto dataIndex = pMesh_->GetPolygonVertex(i, j);
						auto& tex0 = pData[dataIndex].Texture0;

						tex0.x = static_cast<float>(uv[0]);
						tex0.y = static_cast<float>(uv[1]);
					}
				}
			}
		}

		pVertexBuffer_->Unmap(0);
	}

	*pVertexCount_ = controlPointCount;
}

void Mesh::UpdateIndexResources_(Device* pDevice)
{
	SafeDelete(&pIndexBuffer_);
	pIndexBuffer_ = new Resource();

	const auto indexCount = pMesh_->GetPolygonVertexCount();
	pIndexBuffer_->CreateIndexBuffer(pDevice, sizeof(ushort) * indexCount);

	{
		const auto indices = pMesh_->GetPolygonVertices();
		const auto pData = static_cast<ushort*>(pIndexBuffer_->Map(0));

		for (int i = 0; i < indexCount; ++i)
		{
			pData[i] = static_cast<ushort>(indices[i]);
		}

		pIndexBuffer_->Unmap(0);
	}

	*pIndexCount_ = indexCount;
}

