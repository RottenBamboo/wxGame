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

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

struct objConst
{
	matrix TransMatrix;
};

StructuredBuffer<objConst> g_objConst : register(t2);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD, float3 normal : NORMAL)
{
	PSInput result;
	objConst objC = g_objConst[0];
	result.position = mul(lightViewMatrix, position);
	result.uv.x = uv.x;
	result.uv.y = 1.f - uv.y;//v should be vertically reversed because the texture mapping is opposite direction when right hand coordinate transformed to left hand coordinate.
	result.normal = normal;
	return result;
}