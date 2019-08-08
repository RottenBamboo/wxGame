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
	matrix shadowTransform;
	matrix shadowMatrix;
	float4x4 shadowTransformTest;
	float4 cameraPos;
	float4 viewPos;
}

cbuffer cLightmatrix:register(b2)
{
	matrix lightViewMatrix;
	matrix lightPerspectiveMatrix;
	matrix lightRotatMatrix;
	matrix lightShadowTransform;
	matrix lightShadowMatrix;
	float4x4 lightShadowTransformTest;
	float4 lightCameraPos;
	float4 lightViewPos;
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
SamplerState g_samAnisotropicWrap  : register(s1);

void PSMain(PSInput input)
{
	//float4 rgbaColor;// = { 0.f, 0.f, 0.f, 1.f };
	float4 rgbaColor = g_texture.Sample(g_samAnisotropicWrap, input.uv);

}