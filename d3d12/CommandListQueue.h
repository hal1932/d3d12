#pragma once
#include "common.h"
#include <map>
#include <vector>

class CommandList;
class CommandQueue;


class CommandListQueue
{
public:
	~CommandListQueue();

	std::vector<CommandList*>& CreateGroup(const tstring& name, int executionOrder);
	void CommitExecutionOrders();

	std::vector<CommandList*>& GetGroup(const tstring& name);

	bool SetExecutionOrder(int order, const tstring& name);
	void ClearExecutionOrder(const tstring& name);

	void Execute(CommandQueue* pQueue);

private:
	std::map<tstring, int> hashes_;
	std::map<int, int> orders_;
	std::map<int, std::vector<CommandList*>> commandLists_;

	std::vector<std::vector<CommandList*>*> sortedListPtrs_;
};

