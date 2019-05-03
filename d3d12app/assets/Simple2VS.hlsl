#include "SimpleDef.hlsli"

[RootSignature(ROOT_SIGNATURE)]
VSOutput VSFunc(const VSInput input)
{
	VSOutput output = (VSOutput)0;

	float4 localPos = float4(input.Position, 1.0f);
	float4 worldPos = mul(World, localPos);
	float4 viewPos  = mul(View,  worldPos);
	float4 projPos  = mul(Proj,  viewPos);

	output.Position = projPos;
	output.Normal = input.Normal;
	output.Texture0 = input.Texture0;

	return output;
}

