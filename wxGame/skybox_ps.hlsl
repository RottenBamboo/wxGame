#include "common.hlsl"

struct VSOutput
{
	float4 positionH : SV_POSITION;
	float4 position : POSITION1;
};

TextureCube g_texture : register(t0);
SamplerState g_linearWrapSampler : register(s6);

float4 PSMain(VSOutput input) : SV_TARGET
{
	float4 result = g_texture.Sample(g_linearWrapSampler, input.position.xyz);
	return result;
}