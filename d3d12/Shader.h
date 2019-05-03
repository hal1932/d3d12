#pragma once
#include "InputLayout.h"


enum ShaderType
{
	Vertex = D3D12_SHADER_VISIBILITY_VERTEX,
	Hull = D3D12_SHADER_VISIBILITY_HULL,
	Domain = D3D12_SHADER_VISIBILITY_DOMAIN,
	Geometry = D3D12_SHADER_VISIBILITY_GEOMETRY,
	Pixel = D3D12_SHADER_VISIBILITY_PIXEL,
	Compute = -1,
};

struct ShaderDesc
{
	ShaderType Type;
	LPCWSTR FilePath = nullptr;
	LPCTSTR EntryPoint;
	LPCTSTR Profile;

	ShaderDesc(ShaderType type, LPCWSTR filePath, LPCTSTR entryPoint, LPCTSTR profile)
		: Type(type), FilePath(filePath), EntryPoint(entryPoint), Profile(profile)
	{}
};

struct CompiledShaderDesc
{
	ShaderType Type;
	LPCWSTR FilePath = nullptr;

	CompiledShaderDesc(ShaderType type, LPCWSTR filePath)
		: Type(type), FilePath(filePath)
	{}
};


class Device;


class Shader
{
public:
	~Shader();

	ID3DBlob* NativePtr() { return pBlob_.Get(); }
	ShaderType Type() { return type_; }

	D3D12_INPUT_LAYOUT_DESC NativeInputLayout() { return pInputLayout_->NativeObj(); }
	D3D12_SHADER_BYTECODE NativeByteCode() { return CD3DX12_SHADER_BYTECODE(pBlob_.Get()); }

	HRESULT CreateFromSourceFile(const ShaderDesc& desc);
	HRESULT CreateFromCompiledFile(const CompiledShaderDesc& desc);

	HRESULT CreateInputLayout();

private:
	ComPtr<ID3DBlob> pBlob_;
	ShaderType type_;
	std::unique_ptr<InputLayout> pInputLayout_;

	ComPtr<ID3D12RootSignature> pRootSignature_;
};

