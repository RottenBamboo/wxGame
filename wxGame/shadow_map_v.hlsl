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
	objC.TransMatrix = mul(rotatMatrix, objC.TransMatrix);
	result.position = mul(lightOthgraphicMatrix, mul(lightViewMatrix1, mul(objC.TransMatrix, position)));
	return result;
}