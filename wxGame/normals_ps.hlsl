#include "common.hlsl"

struct PSOutput
{
	float4 position : SV_POSITION;
	float4 shadowPosition : POSITION0;
	float4 positionH : POSITION1;
	float4 PositionWorld : POSITION2;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
};

struct objMaterial
{
	float4 mdiffuse;
	float3 mfresnelR0;
	float  mroughness;
};

StructuredBuffer<objMaterial> g_objMaterial : register(t1);

cbuffer csunLight : register(b0)
{
	float3 Strength;
	float  FalloffStart;
	float3 Direction;
	float  FalloffEnd;
	float3 Position;
	float  SpotPower;
}


Texture2D g_texture : register(t0);
Texture2D g_depthMap : register(t4);
Texture2D g_normalMap : register(t3);
SamplerState g_sampler : register(s0);
SamplerState g_samAnisotropicWrap  : register(s1);

float4 PSMain(PSOutput input) : SV_Target
{
	//float4 rgbaColor = g_texture.Sample(g_samAnisotropicWrap, input.uv);
	input.normal = normalize(input.normal);
	float3 normalV = mul((float3x3)viewMatrix, input.normal);
	return float4(normalV, 0.0f);
}