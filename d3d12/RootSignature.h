#pragma once

class Device;
class Shader;


struct RootSignatureDesc
{
	Shader* pVertexShader;
	Shader* pPixelShader;

	RootSignatureDesc(Shader* pVertexShader, Shader* pPixelShader)
		: pVertexShader(pVertexShader), pPixelShader(pPixelShader)
	{}
};


class RootSignature
{
public:
	~RootSignature();

	ID3D12RootSignature* NativePtr() { return pRootSignature_.Get(); }

	HRESULT Create(Device* pDevice, const RootSignatureDesc& desc, UINT staticSamplerCount = 0, const CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = nullptr);

private:
	ComPtr<ID3D12RootSignature> pRootSignature_;
};

