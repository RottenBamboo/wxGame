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
Texture2D g_shadowMap : register(t4);
SamplerState g_sampler : register(s0);
SamplerState gsamAnisotropicWrap  : register(s1);

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 rgbaColor = { 0.1f, 0.1f, 0.1f, 0.9f };	
	rgbaColor *= g_texture.Sample(gsamAnisotropicWrap, input.uv);
	return rgbaColor;
}