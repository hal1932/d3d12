#pragma once
#include "common.h"

class Device;
class Resource;
class CommandList;
class CommandQueue;

class Model
{
public:
	virtual HRESULT UpdateResources(Device* pDevice) = 0;
	virtual HRESULT UpdateSubresources(CommandList* pCommandList, CommandQueue* pCommandQueue) = 0;

public:
	const tstring& Name() { return name_; }
	const tstring& Name() const { return name_; }

	void SetName(const char* name) { name_ = tstring(name); }

private:
	tstring name_;
};
