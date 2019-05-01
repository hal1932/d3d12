#include "stdafx.h"
#include "InputLayout.h"


namespace
{
	DXGI_FORMAT GetElementFormat(const D3D12_SIGNATURE_PARAMETER_DESC& desc)
	{
		if (desc.Mask == 1)
		{
			switch (desc.ComponentType)
			{
				case D3D_REGISTER_COMPONENT_UINT32: return DXGI_FORMAT_R32_UINT;
				case D3D_REGISTER_COMPONENT_SINT32: return DXGI_FORMAT_R32_SINT;
				case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32_FLOAT;
				default: return DXGI_FORMAT_UNKNOWN;
			}
		}
		else if (desc.Mask <= 3)
		{
			switch (desc.ComponentType)
			{
				case D3D_REGISTER_COMPONENT_UINT32: return DXGI_FORMAT_R32G32_UINT;
				case D3D_REGISTER_COMPONENT_SINT32: return DXGI_FORMAT_R32G32_SINT;
				case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32G32_FLOAT;
				default: return DXGI_FORMAT_UNKNOWN;
			}
		}
		else if (desc.Mask <= 7)
		{
			switch (desc.ComponentType)
			{
				case D3D_REGISTER_COMPONENT_UINT32: return DXGI_FORMAT_R32G32B32_UINT;
				case D3D_REGISTER_COMPONENT_SINT32: return DXGI_FORMAT_R32G32B32_SINT;
				case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32G32B32_FLOAT;
				default: return DXGI_FORMAT_UNKNOWN;
			}
		}
		else if (desc.Mask <= 15)
		{
			switch (desc.ComponentType)
			{
				case D3D_REGISTER_COMPONENT_UINT32: return DXGI_FORMAT_R32G32B32A32_UINT;
				case D3D_REGISTER_COMPONENT_SINT32: return DXGI_FORMAT_R32G32B32A32_SINT;
				case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
				default: return DXGI_FORMAT_UNKNOWN;
			}
		}

		return DXGI_FORMAT_UNKNOWN;
	}
}


InputLayout::~InputLayout() {}


HRESULT InputLayout::Create(ID3DBlob* pShaderBlob)
{
	HRESULT result;

	ComPtr<ID3D12ShaderReflection> pRefl;
	result = D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_PPV_ARGS(&pRefl));
	if (FAILED(result))
	{
		return result;
	}

	D3D12_SHADER_DESC shaderDesc;
	result = pRefl->GetDesc(&shaderDesc);
	if (FAILED(result))
	{
		return result;
	}

	elementCount_ = shaderDesc.InputParameters;

	pElements_ = std::make_unique<D3D12_INPUT_ELEMENT_DESC[]>(elementCount_);
	pSemanticNames_ = std::make_unique<std::string[]>(elementCount_);

	for (UINT i = 0; i < elementCount_; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
		result = pRefl->GetInputParameterDesc(i, &paramDesc);
		if (FAILED(result))
		{
			return result;
		}

		pSemanticNames_[i] = std::string(paramDesc.SemanticName);

		auto format = GetElementFormat(paramDesc);

		pElements_[i] =
		{
			pSemanticNames_[i].c_str(),
			paramDesc.SemanticIndex,
			format,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0,
		};
	}

	return result;
}

