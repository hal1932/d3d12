#pragma once
class InputLayout
{
public:
	~InputLayout();

	D3D12_INPUT_LAYOUT_DESC NativeObj() { return { pElements_.get(), elementCount_ }; }
	HRESULT Create(ID3DBlob* pShaderBlob);

private:
	std::unique_ptr<D3D12_INPUT_ELEMENT_DESC[]> pElements_;
	UINT elementCount_ = 0U;
	std::unique_ptr<std::string[]> pSemanticNames_;
};
