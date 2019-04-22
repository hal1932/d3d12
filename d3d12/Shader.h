#pragma once

struct ShaderDesc
{
	LPCWSTR FilePath = nullptr;
	LPCTSTR EntryPoint;
	LPCTSTR Profile;

	ShaderDesc(LPCWSTR filePath, LPCTSTR entryPoint, LPCTSTR profile)
		: FilePath(filePath), EntryPoint(entryPoint), Profile(profile)
	{}
};

struct CompiledShaderDesc
{
	LPCWSTR FilePath = nullptr;

	CompiledShaderDesc(LPCWSTR filePath)
		: FilePath(filePath)
	{}
};

class Shader
{
public:
	~Shader();

	ID3DBlob* NativePtr() { return pBlob_.Get(); }

	D3D12_INPUT_LAYOUT_DESC NativeInputLayout() { return pInputLayout_->NativeObj(); }
	D3D12_SHADER_BYTECODE NativeByteCode() { return CD3DX12_SHADER_BYTECODE(pBlob_.Get()); }

	HRESULT CreateFromSourceFile(const ShaderDesc& desc);
	HRESULT CreateFromCompiledFile(const CompiledShaderDesc& desc);

	HRESULT CreateInputLayout();

private:
	ComPtr<ID3DBlob> pBlob_;

	class InputLayout
	{
	public:
		~InputLayout();

		D3D12_INPUT_LAYOUT_DESC NativeObj() { return { pElements_.get(), elementCount_ }; }
		HRESULT Create(ID3DBlob *pBlob);

	private:
		std::unique_ptr<D3D12_INPUT_ELEMENT_DESC[]> pElements_;
		UINT elementCount_ = 0U;
		std::unique_ptr<std::string[]> pSemanticNames_;
	};
	std::unique_ptr<InputLayout> pInputLayout_;
};

