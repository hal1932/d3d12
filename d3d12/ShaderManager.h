#pragma once
#include "common.h"

namespace fbx
{
	class Model;
}

class Shader;

class ShaderManager
{
public:
	~ShaderManager();

	u64 LoadFromModelMaterial(fbx::Model* pModel);

	const tstring& Name(u64 hash) { return names_[hash]; }

	const D3D12_INPUT_LAYOUT_DESC InputLayout(const tstring& name);

	Shader* VertexShader(const tstring& name);
	const D3D12_SHADER_BYTECODE VertexShaderBytecode(const tstring& name);

	Shader* PixelShader(const tstring& name);
	const D3D12_SHADER_BYTECODE PixelShaderBytecode(const tstring& name);

private:
	std::map<u64, tstring> names_;
	std::map<tstring, Shader*> vertexShaders_;
	std::map<tstring, Shader*> pixelShaders_;
};

