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
	float4 positionH : POSITION1;
	float4 PositionWorld : POSITION2;
	float4 ssaoPosH : POSITION3;
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
	matrix lightOthgraphicMatrix;
	matrix lightViewMatrix1;
	matrix lightTransformNDC;
	matrix invViewMatrix;
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
Texture2D g_ambientmap : register(t6);
SamplerState g_sampler : register(s0);
SamplerState g_linearClampSampler : register(s5);

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
float4 PSMain(PSOutput input) : SV_Target
{ 
	objMaterial objM = g_objMaterial[0]; 

	// Only the first light casts a shadow.
	float3 shadowFactor = float3(1.0f, 1.0f, 0.0f);
	shadowFactor[0] = CalcShadowFactor(input.shadowPosition);

	const float shininess = 1.0f - objM.mroughness;
	float4 rgbaColor = g_texture.Sample(g_samAnisotropicWrap, input.uv);
	float4 NormalMap = g_normalmap.Sample(g_samAnisotropicWrap, input.uv);
	float3 bumpedNormal = NormalSampleToWorldSpace(NormalMap.rgb, input.normal, input.tangentU);
	input.ssaoPosH /= input.ssaoPosH.w;
	float ambientFactor = g_ambientmap.Sample(g_linearClampSampler, input.ssaoPosH.xy, 0.0f).r;
	//float4 ambient = { 0.15f, 0.15f, 0.15f, 1.f };
	float4 ambient = ambientFactor * rgbaColor;
	//ambient = ambient * ambientFactor;// *ambientFactor;
	Material mat = { rgbaColor, objM.mfresnelR0, shininess };
	Light light = { Strength, FalloffStart, Direction, FalloffEnd, Position, SpotPower };
	bumpedNormal = normalize(bumpedNormal);
	float3 cameraPos1 = cameraPos;
	float3 originPos = { 0.f, 0.f, 0.f };
	float3 viewDirection = normalize(cameraPos - input.PositionWorld);
	float3 directLight = ComputeDirectionalLight(light, mat, bumpedNormal, viewDirection)*shadowFactor[0];
	float4 litColor;
	litColor.xyz = directLight + ambient; 
	litColor.w = 0.0f;
	return litColor;
}

