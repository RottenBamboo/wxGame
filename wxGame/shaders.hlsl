#include "lighting.hlsl"
cbuffer cmatrix:register(b2)
{
	matrix linearTransMatrix;
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix finalMatrix;
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

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD, float3 normal : NORMAL)
{
	PSInput result;
	//result.position = mul(perspectiveMatrix, mul(viewMatrix, mul(finalMatrix, mul(linearTransMatrix, position))));
	result.position = mul(finalMatrix, mul(linearTransMatrix, position));
	result.uv.x = uv.x;
	result.uv.y = 1.f - uv.y;//v should be vertically reversed because the texture mapping is opposite direction when right hand coordinate transformed to left hand coordinate.
	result.normal = normal;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{	input.normal = normalize(input.normal);
	float3 cameraPos = { 0.f, -2.f, -50.f };
	float3 originPos = { 0.f, 0.f, 0.f };
	float3 viewDirection = normalize(cameraPos - originPos);

	float4 ambient = { 0.25f, 0.25f, 0.35f, 1.0f };
	ambient = ambient * diffuse;
	const float shininess = 1.0f - roughness;
	float4 rgbaColor = g_texture.Sample(g_sampler, input.uv);
	Material mat = { rgbaColor, fresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	Light light;
	light.Strength = Strength;
	light.FalloffStart = FalloffStart;
	light.Direction = Direction;
	light.FalloffEnd = FalloffEnd;
	light.Position = Position;
	light.SpotPower = SpotPower;
	float3 directLight = ComputeDirectionalLight(light, mat, input.normal, viewDirection);
	float4 finalColor = { directLight + ambient, 1.0f };
	return finalColor;
}