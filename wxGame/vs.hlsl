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
	float4 shadowPosition : POSITION0;
	float4 positionH : POSITION1;
	float4 PositionWorld : POSITION2;
	float4 ssaoPosH : POSITION3;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
}; 

PSOutput VSMain(float4 position : POSITION, float4 uv : TEXCOORD, float3 normal : NORMAL, float3 tangentU : TANGENT)
{
	PSOutput result = (PSOutput)0.0f;
	objConst objC = g_objConst[0];
	objC.TransMatrix = mul(rotatMatrix, objC.TransMatrix);
	result.position = mul(perspectiveMatrix, mul(viewMatrix, mul(objC.TransMatrix, position)));
	result.PositionWorld = mul(objC.TransMatrix, float4(position.xyz, 1.0f));
	result.uv.x = uv.x;
	result.uv.y = 1.f - uv.y;//v should be vertically reversed because the texture mapping is opposite direction when right hand coordinate transformed to left hand coordinate.
	result.normal = mul(objC.TransMatrix, normal);
	result.tangentU = mul(objC.TransMatrix, tangentU);
	float4 posW = mul(objC.TransMatrix, float4(position.xyz, 1.0f));
	float4 posProj = mul(perspectiveMatrix, mul(viewMatrix, posW));	// transform coordinates from view space to texture space
	posProj.x = posProj.x * 0.5f + posProj.w * 0.5f;
	posProj.y = posProj.y * -0.5f + posProj.w * 0.5f;
	result.ssaoPosH = posProj;
	result.positionH = mul(viewMatrix, posW.xyz);
	result.shadowPosition = mul(lightTransformNDC, mul(lightOthgraphicMatrix,mul(lightViewMatrix1, result.PositionWorld)));

	return result;
}