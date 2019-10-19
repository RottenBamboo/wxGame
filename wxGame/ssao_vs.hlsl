cbuffer cmatrix:register(b1)
{
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix rotatMatrix;
	matrix shadowTransform;
	matrix lightOthgraphicMatrix;
	matrix lightViewMatrix1;
	matrix lightTransformNDC;
	matrix invProjMatrix;
	float4 cameraPos;
	float4 viewPos;
}

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
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
};

cbuffer ssaoMatrix:register(b2)
{
	float4 g_offsetVectors[14];
	float4 blurWeights[3];
	float g_occlusionRadius;
	float g_occlusionFadeStart;
	float g_occlusionFadeEnd;
	float g_surfaceEpsilon;
	float DimensionWidth;
	float DimensionHeight;
}

static const float2 g_texCoords[6] =
{
	float2(0.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 1.0f),
	float2(1.0f, 0.0f),
};
//static const float2 g_texCoords[6] =
//{
//	float2(0.0f, 1.0f),
//	float2(0.0f, 0.0f),
//	float2(1.0f, 0.0f),
//	float2(0.0f, 1.0f),
//	float2(1.0f, 0.0f),
//	float2(1.0f, 1.0f)
//};

struct objConst
{
	matrix TransMatrix;
};

StructuredBuffer<objConst> g_objConst : register(t2);

PSOutput VSMain(uint vid : SV_VertexID)
{
	PSOutput result = (PSOutput)0.0f;
	result.uv = g_texCoords[vid];
	result.position = float4(2.0f * result.uv.x - 1.f, 2.f - 2.0f * result.uv.y - 1.0f, 0.0f, 1.0f);
	float4 ph = mul(invProjMatrix, result.position);
	result.positionH.xyz = ph.xyz / ph.w;
	result.positionH.w = 1.f;
	return result;
}