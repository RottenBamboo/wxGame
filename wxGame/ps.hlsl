#include "lighting.hlsl"
struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

cbuffer cmatrix:register(b1)
{
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix rotatMatrix;
	matrix shadowMatrix;
	float4 cameraPos;
	float4 viewPos;
}

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
SamplerState g_sampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{ 
	objMaterial objM = g_objMaterial[0];
	const float shininess = 1.0f - objM.mroughness;
	float4 rgbaColor = g_texture.Sample(g_sampler, input.uv);
	float4 ambient = { 0.15f, 0.15f, 0.15f, 1.f };
	ambient = ambient * rgbaColor;
	Material mat = { rgbaColor, objM.mfresnelR0, shininess };
	Light light = { Strength, FalloffStart, Direction, FalloffEnd, Position, SpotPower };
	input.normal = normalize(input.normal);
	float3 cameraPos1 = cameraPos;
	float3 originPos = { 0.f, 0.f, 0.f };
	float3 viewDirection = normalize(cameraPos1 - viewPos);
	float3 directLight = ComputeDirectionalLight(light, mat, input.normal, viewDirection);
	rgbaColor.xyz = directLight + ambient;
	return rgbaColor;
}

