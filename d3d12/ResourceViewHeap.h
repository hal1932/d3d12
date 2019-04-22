#pragma once

class Device;
class ScreenContext;
class Resource;
class Texture;

struct HeapDesc
{
	enum class ViewType
	{
		RenderTargetView,
		DepthStencilView,
		CbSrUaView, // ConstantBuffer/ShaderResource/UnorderedAccess View
	} ViewType;
	int BufferCount;
};

struct DsvDesc
{
	int Width;
	int Height;
	DXGI_FORMAT Format;
	float ClearDepth;
	unsigned char ClearStencil;
};

struct CsvDesc
{
	int Size;
	D3D12_TEXTURE_LAYOUT Layout;
};

struct SrvDesc
{
	D3D12_SRV_DIMENSION Dimension;
	union
	{
		Texture* pTexture;
	};
};

class ResourceViewHeap
{
public:
	~ResourceViewHeap();

	ID3D12DescriptorHeap* NativePtr() { return pDescriptorHeap_; }

	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle(int index);
	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle(int index);

	HRESULT CreateHeap(Device* pDevice, const HeapDesc& desc);
	void Reset();

	std::vector<Resource*> CreateRenderTargetViewFromBackBuffer(ScreenContext* pScreen);
	Resource* CreateDepthStencilView(ScreenContext* pContext, const DsvDesc& desc);
	Resource* CreateConstantBufferView(const CsvDesc& desc);
	Resource* CreateShaderResourceView(const SrvDesc& desc);

private:
	Device* pDevice_ = nullptr;

	ID3D12DescriptorHeap* pDescriptorHeap_ = nullptr;
	UINT descriptorSize_ = 0U;
	UINT resourceCount_ = 0U;
	int currentSize_ = 0;

	HRESULT CreateHeapImpl_(
		Device* pDevice,
		const HeapDesc& desc,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags);
};

