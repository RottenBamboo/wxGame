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

struct objConst
{
	matrix TransMatrix;
};

StructuredBuffer<objConst> g_objConst : register(t2);

PSOutput VSMain(float4 position : POSITION, float4 uv : TEXCOORD, float3 normal : NORMAL, float3 tangentU : TANGENT)
{
	PSOutput result = (PSOutput)0.0f;
	objConst objC = g_objConst[0];
	result.position = mul(lightOthgraphicMatrix, mul(lightViewMatrix1, mul(objC.TransMatrix, position)));
	result.normal = mul(objC.TransMatrix, normal);
	return result;
}