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

	ulonglong LoadFromModelMaterial(fbx::Model* pModel);

	const tstring& Name(ulonglong hash) { return names_[hash]; }

	const D3D12_INPUT_LAYOUT_DESC InputLayout(const tstring& name);
	const D3D12_SHADER_BYTECODE VertexShader(const tstring& name);
	const D3D12_SHADER_BYTECODE PixelShader(const tstring& name);

private:
	std::map<ulonglong, tstring> names_;
	std::map<tstring, Shader*> vertexShaders_;
	std::map<tstring, Shader*> pixelShaders_;
};

