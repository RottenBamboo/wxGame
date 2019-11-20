#include "common.hlsl"

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
};

static const float4 g_VectorSign[8] =
{
	float4(-1.f, -1.f, -1.f, 1.f),
	float4(1.f, -1.f, -1.f, 1.f),
	float4(1.f, -1.f, 1.f, 1.f),
	float4(-1.f, -1.f, 1.f, 1.f),
	float4(-1.f, 1.f, -1.f, 1.f),
	float4(1.f, 1.f, -1.f, 1.f),
	float4(1.f, 1.f, 1.f, 1.f),
	float4(-1.f, 1.f, 1.f, 1.f),
};

PSOutput VSMain(uint vid : SV_VertexID)
{
	PSOutput result = (PSOutput)0.0f;
	objConst objC = g_objConst[0]; 
	float4 pos = objC.Box.CornerPosition.Vertex[vid].position;
	result.position = mul(perspectiveMatrix, mul(viewMatrix, mul(objC.TransMatrix, pos)));
	return result;
}