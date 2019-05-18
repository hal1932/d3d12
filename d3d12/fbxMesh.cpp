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
	: TransformObject(pMesh)
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

HRESULT Mesh::Setup()
{
	SafeDelete(&pVertexCount_);
	pVertexCount_ = new int();

	SafeDelete(&pIndexCount_);
	pIndexCount_ = new int();

	auto pNode = NativePtr()->GetNode();

	float v[3];
	toFloat3(v, pNode->LclScaling.Get());
	SetScaling(v[0], v[1], v[2]);

	toFloat3Radian(v, pNode->LclRotation.Get());
	SetRotation(v[0], v[1], v[2]);

	toFloat3(v, pNode->LclTranslation.Get());
	SetTranslation(v[0], v[1], v[2]);

	UpdateMatrix();

	SafeDelete(&pMaterial_);
	for (auto i = 0; i < pNode->GetMaterialCount(); ++i)
	{
		auto pMaterial = pNode->GetMaterial(i);
		pMaterial_ = new Material(pMaterial);
		pMaterial_->Setup();
		break;
	}

	return S_OK;
}

HRESULT Mesh::UpdateResources(Device* pDevice)
{
	UpdateVertexResources_(pDevice);
	UpdateIndexResources_(pDevice);

	return pMaterial_->UpdateResources(pDevice);
}

UpdateSubresourceContext* Mesh::UpdateSubresources(CommandList* pCommandList, UpdateSubresourceContext* pContext)
{
	return pMaterial_->UpdateSubresources(pCommandList, pContext);
}

HRESULT Mesh::LoadSkinClusters()
{
	auto pMesh = NativePtr();
	for (auto i = 0; i < pMesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin); ++i)
	{
		auto pSkin = static_cast<FbxSkin*>(pMesh->GetDeformer(i, FbxDeformer::EDeformerType::eSkin));
		skinClusterPtrs_.emplace_back(std::move(std::make_unique<SkinCluster>(pSkin)));
	}
	return S_OK;
}
	
Mesh* Mesh::CreateReference()
{
	auto other = new Mesh(NativePtr());
	other->isReference_ = true;

	other->pVertexBuffer_ = pVertexBuffer_;
	other->pVertexCount_ = pVertexCount_;
	
	other->pIndexBuffer_ = pIndexBuffer_;
	other->pIndexCount_ = pIndexCount_;

	other->pMaterial_ = pMaterial_->CreateReference();

	return other;
}

void Mesh::UpdateVertexResources_(Device* pDevice)
{
	auto pMesh = NativePtr();

	SafeDelete(&pVertexBuffer_);
	pVertexBuffer_ = new Resource();

	const auto controlPointCount = pMesh->GetControlPointsCount();
	pVertexBuffer_->CreateVertexBuffer(pDevice, sizeof(Vertex) * controlPointCount);

	{
		const auto pData = static_cast<Vertex*>(pVertexBuffer_->Map(0));

		// 位置
		{
			const auto controlPoints = pMesh->GetControlPoints();

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

			for (int i = 0; i < pMesh->GetPolygonCount(); ++i)
			{
				for (int j = 0; j < pMesh->GetPolygonSize(i); ++j)
				{
					const auto dataIndex = pMesh->GetPolygonVertex(i, j);

					FbxVector4 normal;
					if (pMesh->GetPolygonVertexNormal(i, j, normal))
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
			for (int i = 0; i < pMesh->GetPolygonCount(); ++i)
			{
				for (int j = 0; j < pMesh->GetPolygonSize(i); ++j)
				{
					FbxVector2 uv;
					bool unmapped;
					if (pMesh->GetPolygonVertexUV(i, j, "map1", uv, unmapped))
					{
						const auto dataIndex = pMesh->GetPolygonVertex(i, j);
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
	auto pMesh = NativePtr();

	SafeDelete(&pIndexBuffer_);
	pIndexBuffer_ = new Resource();

	const auto indexCount = pMesh->GetPolygonVertexCount();
	pIndexBuffer_->CreateIndexBuffer(pDevice, sizeof(u16) * indexCount);

	{
		const auto indices = pMesh->GetPolygonVertices();
		const auto pData = static_cast<u16*>(pIndexBuffer_->Map(0));

		for (int i = 0; i < indexCount; ++i)
		{
			pData[i] = static_cast<u16>(indices[i]);
		}

		pIndexBuffer_->Unmap(0);
	}

	*pIndexCount_ = indexCount;
}

