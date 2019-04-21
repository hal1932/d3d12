#pragma once
#include <d3d12.h>

class Device;
class CommandList;
class Resource;

HRESULT UpdateSubresources(
	Device* pDevice,
	const D3D12_SUBRESOURCE_DATA* pData, int firstSubresource, int subresourceCount,
	Resource* pDestinationResource,
	CommandList* pDestinationCommandList,
	Resource* pIntermediateResource);