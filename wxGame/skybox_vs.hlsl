#include "common.hlsl"

struct PSInput
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
};

struct VSOutput
{
	float4 positionH : SV_POSITION;
	float4 position : POSITION1;
};

VSOutput VSMain(float4 position : POSITION, float4 uv : TEXCOORD, float3 normal : NORMAL, float3 tangentU : TANGENT)
{
	VSOutput result = (VSOutput)0.0f;

	objConst objC = g_objConst[0];
	//cubemap lookup vector. the position value is the lookup vector because it already  normalized.
	result.position.xyz = position.xyz;
	float4 positionWorld = mul(objC.TransMatrix, float4(result.position.xyz, 1.f));
	positionWorld.xyz += cameraPos;
	result.positionH = mul(perspectiveMatrix, mul(viewMatrix, positionWorld)).xyww;

	return result;
}