#include "common.hlsl"

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD, float3 normal : NORMAL)
{
	PSInput result;
	objConst objC = g_objConst[0];
	objC.TransMatrix = mul(rotatMatrix, objC.TransMatrix);
	result.position = mul(lightOthgraphicMatrix, mul(lightViewMatrix1, mul(objC.TransMatrix, position)));
	return result;
}