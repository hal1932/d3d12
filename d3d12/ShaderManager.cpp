#include "ShaderManager.h"
#include "fbxModel.h"
#include "fbxMesh.h"
#include "fbxMaterial.h"
#include "Shader.h"
#include <tchar.h>
#include <memory>

ShaderManager::~ShaderManager()
{
	for (auto& vs : vertexShaders_)
	{
		SafeDelete(&vs.second);
	}

	for (auto& ps : pixelShaders_)
	{
		SafeDelete(&ps.second);
	}
}

ulonglong ShaderManager::LoadFromModelMaterial(fbx::Model* pModel)
{
	// TODO: エラーハンドリング
	const auto& name = pModel->MeshPtr(0)->MaterialPtr()->Name();

	Shader* pVS;
	Shader* pPS;

	auto pVSIter = vertexShaders_.find(name);
	if (pVSIter != vertexShaders_.end())
	{
		pVS = pVSIter->second;
	}
	else
	{
		auto pVertexShader = new Shader();
		auto path = tstring_to_wcs("assets/" + name + "VS.hlsl");
		pVertexShader->CreateFromSourceFile({ ShaderType::Vertex, path, _T("VSFunc"), _T("vs_5_0") });
		SafeDeleteArray(&path);

		pVertexShader->CreateInputLayout();

		vertexShaders_[name] = pVertexShader;
		pVS = pVertexShader;
	}

	auto pPsIter = pixelShaders_.find(name);
	if (pPsIter != pixelShaders_.end())
	{
		pPS = pPsIter->second;
	}
	else
	{
		auto pPixelShader = new Shader();
		auto path = tstring_to_wcs("assets/" + name + "PS.hlsl");
		pPixelShader->CreateFromSourceFile({ ShaderType::Pixel, path, _T("PSFunc"), _T("ps_5_0") });
		SafeDeleteArray(&path);

		pixelShaders_[name] = pPixelShader;
		pPS = pPixelShader;
	}

	auto shaderHash = 0UL;
	shaderHash ^= reinterpret_cast<ulonglong>(pVS);
	shaderHash ^= reinterpret_cast<ulonglong>(pPS);
	pModel->SetShaderHash(shaderHash);

	names_[shaderHash] = name;

	return shaderHash;
}

const D3D12_INPUT_LAYOUT_DESC ShaderManager::InputLayout(const tstring& name)
{
	return vertexShaders_[name]->NativeInputLayout();
}

Shader* ShaderManager::VertexShader(const tstring& name)
{
	return vertexShaders_[name];
}

const D3D12_SHADER_BYTECODE ShaderManager::VertexShaderBytecode(const tstring& name)
{
	return VertexShader(name)->NativeByteCode();
}

Shader* ShaderManager::PixelShader(const tstring& name)
{
	return pixelShaders_[name];
}

const D3D12_SHADER_BYTECODE ShaderManager::PixelShaderBytecode(const tstring& name)
{
	return PixelShader(name)->NativeByteCode();
}
