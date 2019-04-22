#pragma once
class Device;
class CommandContainer;

class CommandList
{
public:
	enum class SubmitType
	{
		Direct = D3D12_COMMAND_LIST_TYPE_DIRECT,
		Bundle = D3D12_COMMAND_LIST_TYPE_BUNDLE,
	};

public:
	~CommandList();

	SubmitType Type() { return type_; }
	ID3D12CommandList* NativePtr() { return pNativeList_.Get(); }
	ID3D12GraphicsCommandList* GraphicsList() { return static_cast<ID3D12GraphicsCommandList*>(pNativeList_.Get()); }

	HRESULT Create(Device* pDevice, SubmitType type, int bufferCount);
	HRESULT Open(ID3D12PipelineState* pPipelineState, bool swapBuffers = true);
	void Close();

private:
	SubmitType type_;
	ComPtr<ID3D12CommandList> pNativeList_;
	std::vector<ComPtr<ID3D12CommandAllocator>> allocatorPtrs_;
	int allocatorCount_ = 0;
	int currentAllocatorIndex_ = 0;
};

