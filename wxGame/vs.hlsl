cbuffer cmatrix:register(b1)
{
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix rotatMatrix;
	matrix shadowTransform;
	matrix lightOthgraphicMatrix;
	matrix lightViewMatrix1;
	matrix lightTransformNDC;
	matrix shadowMatrix;
	float4x4 shadowTransformTest;
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
	float3 PositionWorld : POSITION2;
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
	result.position = mul(perspectiveMatrix, mul(viewMatrix, mul(objC.TransMatrix, position)));
	//result.position = mul(lightOthgraphicMatrix, mul(lightViewMatrix1, mul(lightTransformNDC, (objC.TransMatrix, position))));
	//result.position = mul(shadowMatrix, mul(objC.TransMatrix, position));
	result.PositionWorld = mul(objC.TransMatrix, position);
	result.uv.x = uv.x;
	result.uv.y = 1.f - uv.y;//v should be vertically reversed because the texture mapping is opposite direction when right hand coordinate transformed to left hand coordinate.
	result.normal = mul(objC.TransMatrix, normal);
	result.tangentU = mul(objC.TransMatrix, tangentU);
	float4 posW = mul(objC.TransMatrix, float4(position.xyz, 1.0f));
	result.positionH = mul(viewMatrix, posW.xyz);
	result.shadowPosition = mul(mul(lightTransformNDC,lightOthgraphicMatrix),mul(lightViewMatrix1, result.PositionWorld));//PositionWorld's direction is opposite to the sun light.
	//result.shadowPosition = mul(shadowMatrix, result.PositionWorld);
	result.shadowPosition.w = 1.0f;

	return result;
}