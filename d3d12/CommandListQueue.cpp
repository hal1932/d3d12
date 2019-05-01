#include "CommandListQueue.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include <ctime>

CommandListQueue::~CommandListQueue()
{
	for (auto& lists : commandLists_)
	{
		for (auto pList : lists.second)
		{
			SafeDelete(&pList);
		}
	}
}

std::vector<CommandList*>& CommandListQueue::CreateGroup(const tstring& name, int executionOrder)
{
	int hash;

	const auto hashIter = hashes_.find(name);
	if (hashIter != hashes_.end())
	{
		hash = hashIter->second;
		for (auto& oldList : commandLists_[hash])
		{
			SafeDelete(&oldList);
		}
	}
	else
	{
		hash = static_cast<int>(clock());
		while (commandLists_.find(hash) != commandLists_.end())
		{
			hash = static_cast<int>(clock());
		}
	}

	hashes_[name] = hash;
	orders_[hash] = executionOrder;	
	commandLists_[hash] = std::vector<CommandList*>();

	return commandLists_[hash];
}

void CommandListQueue::CommitExecutionOrders()
{
	sortedListPtrs_.clear();

	for (auto orderIter : orders_)
	{
		const auto order = orderIter.second;
		if (order < 0)
		{
			continue;
		}

		const auto hash = orderIter.first;
		sortedListPtrs_.push_back(&commandLists_[hash]);
	}
}

std::vector<CommandList*>& CommandListQueue::GetGroup(const tstring& name)
{
	const auto hashIter = hashes_.find(name);
	if (hashIter == hashes_.end())
	{
		throw "command list not found";
	}

	return commandLists_[hashIter->second];
}

bool CommandListQueue::SetExecutionOrder(int order, const tstring& name)
{
	const auto hashIter = hashes_.find(name);
	if (hashIter == hashes_.end())
	{
		return false;
	}

	orders_[hashIter->second] = order;
	return true;
}

void CommandListQueue::ClearExecutionOrder(const tstring& name)
{
	SetExecutionOrder(-1, name);
}

void CommandListQueue::Execute(CommandQueue* pQueue)
{
	auto pNativeQueue = pQueue->NativePtr();

	for (auto pLists : sortedListPtrs_)
	{
		if (pLists->size() == 0)
		{
			continue;
		}

		std::vector<ID3D12CommandList*> tmp(pLists->size());
		for (auto i = 0; i < tmp.size(); ++i)
		{
			tmp[i] = (*pLists)[i]->NativePtr();
		}

		pNativeQueue->ExecuteCommandLists(static_cast<UINT>(tmp.size()), tmp.data());
	}
}
