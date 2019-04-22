#pragma once

struct ResourceDesc
{
	D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT;
	DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
	D3D12_RESOURCE_DIMENSION Dimension;
	UINT64 Width;
	UINT Height = 1;
	short Depth = 1;
	short MipLevels = 1;
	int SampleCount = 1;
	int SampleQuality = 0;
	D3D12_TEXTURE_LAYOUT Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_RESOURCE_STATES States;

	static inline ResourceDesc Buffer(
		D3D12_RESOURCE_STATES initialState,
		UINT64 sizeInBytes, D3D12_TEXTURE_LAYOUT layout)
	{
		ResourceDesc desc = {};
		desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = sizeInBytes;
		desc.Layout = layout;
		desc.States = initialState;
		return desc;
	}

	static inline ResourceDesc Tex2D(
		DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState,
		UINT64 width, UINT height, UINT16 arraySize = 1,
		UINT16 mipLevels = 1,
		UINT sampleCount = 1, UINT sampleQuality = 0,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN)
	{
		ResourceDesc desc = {};
		desc.Format = format;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = width;
		desc.Height = height;
		desc.Depth = arraySize;
		desc.MipLevels = mipLevels;
		desc.SampleCount = sampleCount;
		desc.SampleQuality = sampleQuality;
		desc.Layout = layout;
		desc.Flags = flags;
		desc.States = initialState;
		return desc;
	}
};
