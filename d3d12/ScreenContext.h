#pragma once

class Device;
class CommandQueue;

struct ScreenContextDesc
{
	int BufferCount;
	DXGI_FORMAT Format;
	int Width;
	int Height;
	HWND OutputWindow;
	bool Windowed;

	ScreenContextDesc()
		: BufferCount(2),
		Format(DXGI_FORMAT_UNKNOWN),
		Width(0), Height(0),
		OutputWindow(nullptr),
		Windowed(true)
	{}
};

class ScreenContext
{
public:
	~ScreenContext();

	UINT FrameIndex() { return frameIndex_; }
	const ScreenContextDesc& Desc() { return desc_; }

	const int Width() { return desc_.Width; }
	const int Height() { return desc_.Height; }
	const float AspectRatio() { return (float)Width() / (float)Height(); }

	void Create(Device* pDevice, CommandQueue* pCommandQueue, const ScreenContextDesc& desc);
	void Reset();

	void UpdateFrameIndex();
	void SwapBuffers(int syncInterval);

public: // internal
	HRESULT GetBackBufferView(UINT index, ID3D12Resource** ppView);

private:
	Device* pDevice_ = nullptr;

	ScreenContextDesc desc_;
	IDXGISwapChain3* pSwapChain_ = nullptr;
	
	UINT frameIndex_ = 0U;
};

