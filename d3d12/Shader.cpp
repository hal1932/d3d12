#include "Shader.h"
#include "common.h"
#include <D3Dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace
{
	DXGI_FORMAT GetElementFormat(const D3D12_SIGNATURE_PARAMETER_DESC& desc);
}

Shader::~Shader() {}
HRESULT Shader::CreateFromSourceFile(const ShaderDesc& desc)
{
	HRESULT result;

	auto flags = 0U;
#if _DEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	ComPtr<ID3DBlob> error;
	result = D3DCompileFromFile(
		desc.FilePath,
		nullptr, // D3D_SHADER_MACRO* pDefines
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		desc.EntryPoint,
		desc.Profile,
		flags,
		0,
		&pBlob_,
		&error);
	if (FAILED(result))
	{
		return result;
	}

	return result;
}

HRESULT Shader::CreateFromCompiledFile(const CompiledShaderDesc& desc)
{
	HRESULT result;

	result = D3DReadFileToBlob(desc.FilePath, &pBlob_);
	if (FAILED(result))
	{
		return result;
	}

	return result;
}

HRESULT Shader::CreateInputLayout()
{
	pInputLayout_ = std::make_unique<InputLayout>();
	return pInputLayout_->Create(pBlob_.Get());
}

Shader::InputLayout::~InputLayout() {}

HRESULT Shader::InputLayout::Create(ID3DBlob* pBlob)
{
	HRESULT result;

	ComPtr<ID3D12ShaderReflection> pRefl;
	result = D3DReflect(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(&pRefl));
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