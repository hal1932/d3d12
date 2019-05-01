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

	type_ = desc.Type;

	auto flags = 0U;
#if _DEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	ComPtr<ID3DBlob> pError;
	result = D3DCompileFromFile(
		desc.FilePath,
		nullptr, // D3D_SHADER_MACRO* pDefines
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		desc.EntryPoint,
		desc.Profile,
		flags,
		0,
		&pBlob_,
		&pError);
	if (FAILED(result))
	{
		auto message = reinterpret_cast<char*>(pError->GetBufferPointer());
		OutputDebugStringA(message);
		return result;
	}

	return result;
}

HRESULT Shader::CreateFromCompiledFile(const CompiledShaderDesc& desc)
{
	HRESULT result;

	type_ = desc.Type;

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
