#include "UpdateSubresources.h"
#include "common.h"
#include "Device.h"
#include "CommandList.h"
#include "Resource.h"
#include "d3dx12.h"
#include <Windows.h>

UINT64 GetSubresourcesFootprint_(ID3D12Device* pDevice, int start, int count, const D3D12_RESOURCE_DESC& desc);

HRESULT UpdateSubresourcesImpl_(
	ID3D12Device* pDevice,
	const D3D12_SUBRESOURCE_DATA* pData,
	ID3D12GraphicsCommandList* pCommandList,
	Resource* pDestResource, const D3D12_RESOURCE_DESC& destResourceDesc,
	ID3D12Resource* pIntermediate,
	UINT64 offset, UINT start, UINT count);

HRESULT UpdateSubresourcesImpl_(
	const D3D12_SUBRESOURCE_DATA* pData,
	ID3D12GraphicsCommandList* pCommandList,
	ID3D12Resource* pDestResource, const D3D12_RESOURCE_DESC& destResourceDesc,
	ID3D12Resource* pIntermediate,
	UINT start, UINT count,
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pRowCounts, UINT64* pRowSizesInBytes);

void CopySubresource_(const D3D12_MEMCPY_DEST* pDest, const D3D12_SUBRESOURCE_DATA* pSource, UINT64 rowSizeInBytes, UINT rowCount, UINT sliceCount);

HRESULT UpdateSubresources(
	Device* pDevice,
	const D3D12_SUBRESOURCE_DATA* pData, int firstSubresource, int subresourceCount,
	Resource* pDestinationResource,
	CommandList* pDestinationCommandList,
	Resource* pIntermediateResource)
{
	HRESULT result;

	auto pNativeDevice = pDevice->NativePtr();
	const auto destResourceDesc = pDestinationResource->NativePtr()->GetDesc();

	const auto intermediateDesc = ResourceDesc::Buffer(
		D3D12_RESOURCE_STATE_GENERIC_READ,
		GetSubresourcesFootprint_(pNativeDevice, firstSubresource, subresourceCount, destResourceDesc),
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR);

	result = pIntermediateResource->CreateCommited(pDevice, intermediateDesc);
	if (FAILED(result))
	{
		return result;
	}

	auto pNativeCommandList = pDestinationCommandList->GraphicsList();

	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			pDestinationResource->NativePtr(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		pNativeCommandList->ResourceBarrier(1, &barrier);
	}

	result = UpdateSubresourcesImpl_(
		pNativeDevice,
		pData,
		pNativeCommandList,
		pDestinationResource, destResourceDesc,
		pIntermediateResource->NativePtr(),
		0, firstSubresource, subresourceCount);
	if (FAILED(result))
	{
		return result;
	}

	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			pDestinationResource->NativePtr(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		pNativeCommandList->ResourceBarrier(1, &barrier);
	}

	return result;
}

UINT64 GetSubresourcesFootprint_(ID3D12Device* pDevice, int start, int count, const D3D12_RESOURCE_DESC& desc)
{
	UINT64 result;
	pDevice->GetCopyableFootprints(
		&desc,
		start, count,
		0, // UINT64 BaseOffset
		nullptr, // D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pLayouts
		nullptr, // UINT *pNumRows
		nullptr, // UINT64 *pRowSizeInBytes
		&result
	);

	return result;
}

HRESULT UpdateSubresourcesImpl_(
	ID3D12Device* pDevice,
	const D3D12_SUBRESOURCE_DATA* pData,
	ID3D12GraphicsCommandList* pCommandList,
	Resource* pDestResource, const D3D12_RESOURCE_DESC& destResourceDesc,
	ID3D12Resource* pIntermediate,
	UINT64 offset, UINT start, UINT count)
{
	HRESULT result;

	const auto singleBufferSize = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
	const auto bufferSize = singleBufferSize * count;

	if (bufferSize > SIZE_MAX)
	{
		return S_FALSE;
	}

	auto buffer = HeapAlloc(GetProcessHeap(), 0, static_cast<size_t>(bufferSize));
	if (!buffer)
	{
		return S_FALSE;
	}

	auto pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(buffer);
	auto pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + count);
	auto pRowCounts = reinterpret_cast<UINT*>(pRowSizesInBytes + count);

	UINT64 requiredSize;
	pDevice->GetCopyableFootprints(
		&destResourceDesc,
		start, count,
		offset,
		pLayouts, pRowCounts, pRowSizesInBytes,
		&requiredSize);

	result = UpdateSubresourcesImpl_(
		pData,
		pCommandList,
		pDestResource->NativePtr(), destResourceDesc,
		pIntermediate,
		start, count,
		pLayouts, pRowCounts, pRowSizesInBytes);

	HeapFree(GetProcessHeap(), 0, buffer);

	return result;
}

HRESULT UpdateSubresourcesImpl_(
	const D3D12_SUBRESOURCE_DATA* pData,
	ID3D12GraphicsCommandList* pCommandList,
	ID3D12Resource* pDestResource, const D3D12_RESOURCE_DESC& destResourceDesc,
	ID3D12Resource* pIntermediate,
	UINT start, UINT count,
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pRowCounts, UINT64* pRowSizesInBytes)
{
	HRESULT result;

	{
		void* pMappedData;
		result = pIntermediate->Map(0, nullptr, &pMappedData);
		if (FAILED(result))
		{
			return result;
		}

		for (UINT i = 0; i < count; ++i)
		{
			D3D12_MEMCPY_DEST dest = {};
			dest.pData = reinterpret_cast<byte*>(pMappedData) + pLayouts[i].Offset;
			dest.RowPitch = pLayouts[i].Footprint.RowPitch;
			dest.SlicePitch = pLayouts[i].Footprint.RowPitch * pRowCounts[i];
			CopySubresource_(&dest, &pData[i], pRowSizesInBytes[i], pRowCounts[i], pLayouts[i].Footprint.Depth);
		}

		pIntermediate->Unmap(0, nullptr);
	}

	if (destResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		CD3DX12_BOX box(
			static_cast<UINT>(pLayouts[0].Offset),
			static_cast<UINT>(pLayouts[0].Offset + pLayouts[0].Footprint.Width));

		pCommandList->CopyBufferRegion(
			pDestResource, 0,
			pIntermediate, pLayouts[0].Offset,
			pLayouts[0].Footprint.Width);
	}
	else
	{
		for (UINT i = 0; i < count; ++i)
		{
			CD3DX12_TEXTURE_COPY_LOCATION src(pIntermediate, pLayouts[i]);
			CD3DX12_TEXTURE_COPY_LOCATION dst(pDestResource, start + i);
			pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
	}

	return result;
}

void CopySubresource_(const D3D12_MEMCPY_DEST* pDest, const D3D12_SUBRESOURCE_DATA* pSource, UINT64 rowSizeInBytes, UINT rowCount, UINT sliceCount)
{
	for (UINT i = 0; i < sliceCount; ++i)
	{
		auto pDestSlice = reinterpret_cast<byte*>(pDest->pData) + pDest->SlicePitch * i;
		const auto pSrcSlice = reinterpret_cast<const byte*>(pSource->pData) + pSource->SlicePitch * i;

		for (UINT j = 0; j < rowCount; ++j)
		{
			memcpy(
				pDestSlice + pDest->RowPitch * j,
				pSrcSlice + pSource->RowPitch * j,
				rowSizeInBytes);
		}
	}
}
