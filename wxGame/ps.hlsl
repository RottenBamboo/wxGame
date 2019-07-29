#include "lighting.hlsl"
struct PSInput
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
};

struct PSOutput
{
	float4 position : SV_POSITION;
	float4 shadowPosition : POSITION0;
	float3 PositionWorld : POSITION1;
	float4 positionH : POSITION2;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
};
cbuffer cmatrix:register(b1)
{
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix rotatMatrix;
	matrix shadowTransform;
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
Texture2D g_normalmap : register(t3);
SamplerState g_sampler : register(s0);

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}
float4 PSMain(PSOutput input) : SV_TARGET
{ 
	objMaterial objM = g_objMaterial[0]; 

	// Only the first light casts a shadow.
	float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
	shadowFactor[0] = CalcShadowFactor(input.shadowPosition);

	const float shininess = 1.0f - objM.mroughness;
	float4 rgbaColor = g_texture.Sample(g_sampler, input.uv);
	float4 NormalMap = g_normalmap.Sample(g_sampler, input.uv);
	float3 bumpedNormal = NormalSampleToWorldSpace(NormalMap.rgb, input.normal, input.tangentU);
	float4 ambient = { 0.15f, 0.15f, 0.15f, 1.f };
	ambient = ambient * rgbaColor;
	Material mat = { rgbaColor, objM.mfresnelR0, shininess };
	Light light = { Strength, FalloffStart, Direction, FalloffEnd, Position, SpotPower };
	//bumpedNormal = normalize(input.normal);
	bumpedNormal = normalize(bumpedNormal);
	float3 cameraPos1 = cameraPos;
	float3 originPos = { 0.f, 0.f, 0.f };
	float3 viewDirection = normalize(cameraPos1 - input.positionH);
	//float3 directLight = ComputeDirectionalLight(light, mat, input.normal, viewDirection);
	float3 directLight = ComputeDirectionalLight(light, mat, bumpedNormal, viewDirection) *shadowFactor[0];
	float4 litColor;
	litColor.xyz = directLight + ambient;
	return litColor;
}

