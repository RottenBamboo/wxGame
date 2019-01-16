#include "lighting.hlsl"
struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : TEXCOORD3;
};

cbuffer cmatrix:register(b2)
{
	matrix linearTransMatrix;
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix rotatMatrix;
	float4 cameraPos;
	float4 viewPos;
}

cbuffer cmaterial:register(b0)
{
	float4 diffuse;
	float3 fresnelR0;
	float  roughness;
}

cbuffer csunLight : register(b1)
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
	const float shininess = 1.0f - roughness;
	float4 rgbaColor = g_texture.Sample(g_sampler, input.uv);
	float4 ambient = { 0.35f, 0.35f, 0.35f, 1.f };
	ambient = ambient * rgbaColor;
	Material mat = { rgbaColor, fresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	Light light = { Strength, FalloffStart, Direction, FalloffEnd, Position, SpotPower };
	input.normal = normalize(input.normal);
	float3 cameraPos1 = cameraPos;
	float3 originPos = { 0.f, 0.f, 0.f };
	float3 viewDirection = normalize(cameraPos1 - viewPos);

	float3 directLight = ComputeDirectionalLight(light, mat, input.normal, viewDirection);
	float4 finalColor = { directLight + ambient, 1.0f };
	return finalColor;
}

