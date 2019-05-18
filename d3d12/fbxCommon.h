#pragma once
#include "common.h"
#include <fbxsdk.h>
#include <functional>
#include <stack>
#include <queue>

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

	inline void TraverseDepthFirst(FbxNode* pRootNode, FbxNodeAttribute::EType type, std::function<bool(FbxNode*)> callback)
	{
		auto stack = std::stack<FbxNode*>();
		stack.push(pRootNode);

		while (!stack.empty())
		{
			auto pNode = stack.top();
			stack.pop();

			auto continueTraversing = true;

			const auto pAttribute = pNode->GetNodeAttribute();
			if (pAttribute != nullptr)
			{
				const auto attrType = pAttribute->GetAttributeType();
				if (attrType == type)
				{
					continueTraversing = callback(pNode);
				}
			}

			if (continueTraversing)
			{
				for (auto i = 0; i < pNode->GetChildCount(); ++i)
				{
					stack.push(pNode->GetChild(i));
				}
			}
		}
	}

	inline void TraverseBreadthFirst(FbxNode* pRootNode, FbxNodeAttribute::EType type, std::function<bool(FbxNode*)> callback)
	{
		auto queue = std::queue<FbxNode*>();
		queue.push(pRootNode);

		while (!queue.empty())
		{
			auto pNode = queue.front();
			queue.pop();

			auto continueTraversing = true;

			const auto pAttribute = pNode->GetNodeAttribute();
			if (pAttribute != nullptr)
			{
				const auto attrType = pAttribute->GetAttributeType();
				if (attrType == type)
				{
					continueTraversing = callback(pNode);
				}
			}

			if (continueTraversing)
			{
				for (auto i = 0; i < pNode->GetChildCount(); ++i)
				{
					queue.push(pNode->GetChild(i));
				}
			}
		}
	}

}// namespace fbx
