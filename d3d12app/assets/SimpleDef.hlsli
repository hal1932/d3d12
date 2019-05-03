struct VSInput
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float2 Texture0 : TEXTURE0;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL;
	float2 Texture0 : TEXTURE0;
};

struct PSOutput
{
	float4 Color   : SV_TARGET0;
};


cbuffer Model : register(b0)
{
	float4x4 World : packoffset(c0);
};

cbuffer Camera : register(b1)
{
	float4x4 View  : packoffset(c0);
	float4x4 Proj  : packoffset(c4);
};

Texture2D ColorTexture : register(t0);
SamplerState ColorSampler : register(s0);


#define ROOT_SIGNATURE \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
	"DescriptorTable(CBV(b0), Visibility = SHADER_VISIBILITY_VERTEX), " \
	"DescriptorTable(CBV(b1), Visibility = SHADER_VISIBILITY_VERTEX), " \
	"DescriptorTable(SRV(t0), Visibility = SHADER_VISIBILITY_PIXEL), " \
	"StaticSampler(s0, Filter = FILTER_MIN_MAG_MIP_LINEAR)"
