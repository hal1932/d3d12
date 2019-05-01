#include "RootSignature.h"
#include "Device.h"
#include "Shader.h"


namespace
{
	struct RootParam
	{
		D3D12_DESCRIPTOR_RANGE_TYPE RangeType;
		UINT BaseShaderRegister;
		UINT RegisterSpace;
		D3D12_SHADER_VISIBILITY ShaderVisibility;
	};

	HRESULT CreateRootParams(
		std::vector<RootParam>* pOut,
		const D3D12_SHADER_BYTECODE& bytecode,
		D3D12_SHADER_VISIBILITY visibility,
		UINT staticSamplerCount,
		const CD3DX12_STATIC_SAMPLER_DESC* staticSamplers)
	{
		auto result = S_OK;

		ComPtr<ID3D12ShaderReflection> pRefl;
		result = D3DReflect(bytecode.pShaderBytecode, bytecode.BytecodeLength, IID_PPV_ARGS(&pRefl));
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

		for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
		{
			D3D12_SHADER_INPUT_BIND_DESC desc;
			pRefl->GetResourceBindingDesc(i, &desc);

			auto rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			switch (desc.Type)
			{
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
					rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					break;

				case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
					rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					break;

				case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_TBUFFER:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_BYTEADDRESS:
					rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					break;

				case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_APPEND_STRUCTURED:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_CONSUME_STRUCTURED:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWBYTEADDRESS:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED:
					rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					break;

				default:
					return E_INVALIDARG;
			}

			const auto shaderRegister = desc.BindPoint;

			if (rangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			{
				auto skip = false;
				for (UINT i = 0; i < staticSamplerCount; ++i)
				{
					if (staticSamplers[i].ShaderRegister == shaderRegister)
					{
						skip = true;
						break;
					}
				}
				if (skip)
				{
					continue;
				}
			}

			RootParam param;
			param.RangeType = rangeType;
			param.BaseShaderRegister = shaderRegister;
			param.RegisterSpace = desc.Space;
			param.ShaderVisibility = visibility;
			pOut->push_back(param);
		}

		return result;
	}
}


RootSignature::~RootSignature() {}


HRESULT RootSignature::Create(Device* pDevice, const RootSignatureDesc& desc, UINT staticSamplerCount, const CD3DX12_STATIC_SAMPLER_DESC* staticSamplers)
{
	std::vector<RootParam> rootParams;
	D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	HRESULT result;

	if (desc.pVertexShader != nullptr)
	{
		result = CreateRootParams(&rootParams, desc.pVertexShader->NativeByteCode(), D3D12_SHADER_VISIBILITY_VERTEX, staticSamplerCount, staticSamplers);
		if (FAILED(result))
		{
			return result;
		}
		flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flags ^= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	}

	if (desc.pPixelShader != nullptr)
	{
		result = CreateRootParams(&rootParams, desc.pPixelShader->NativeByteCode(), D3D12_SHADER_VISIBILITY_PIXEL, staticSamplerCount, staticSamplers);
		if (FAILED(result))
		{
			return result;
		}
		flags ^= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	}

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(rootParams.size());
	std::vector<CD3DX12_ROOT_PARAMETER1> params(rootParams.size());
	for (auto i = 0; i < rootParams.size(); ++i)
	{
		const auto& param = rootParams[i];
		ranges[i].Init(param.RangeType, 1, param.BaseShaderRegister, param.RegisterSpace);
		params[i].InitAsDescriptorTable(1, &ranges[i], param.ShaderVisibility);
	}

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rawDesc;
	rawDesc.Init_1_1(
		static_cast<UINT>(params.size()), &params[0],
		staticSamplerCount, staticSamplers,
		flags
	);

	ComPtr<ID3DBlob> pSignature;
	ComPtr<ID3DBlob> pError;

	result = D3DX12SerializeVersionedRootSignature(
			&rawDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&pSignature,
			&pError);
	if (FAILED(result))
	{
		auto message = reinterpret_cast<char*>(pError->GetBufferPointer());
		OutputDebugStringA(message);
		return result;
	}

	auto pNativeDevice = pDevice->NativePtr();
	result = pNativeDevice->CreateRootSignature(
			0,
			pSignature->GetBufferPointer(),
			pSignature->GetBufferSize(),
			IID_PPV_ARGS(&pRootSignature_));
	if (FAILED(result))
	{
		return result;
	}

	return result;
}
