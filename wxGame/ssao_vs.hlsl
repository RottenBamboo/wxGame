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
static const float2 g_texCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f)
};
struct objConst
{
	matrix TransMatrix;
};

StructuredBuffer<objConst> g_objConst : register(t2);

PSOutput VSMain(uint vid : SV_VertexID)
{
	PSOutput result = (PSOutput)0.0f;
	result.uv = g_texCoords[vid];
	result.positionH = float4(2.0f*result.uv.x - 1.0f, 1.0f - 2.0f * result.uv.y, 0.0f, 1.0f);
	float4 ph = mul(invViewMatrix, result.positionH);
	result.position.xyz = ph.xyz / ph.w;
	result.position.w = 1.f;
	return result;
}