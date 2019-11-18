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
};
struct objConst
{
	matrix TransMatrix;
};

StructuredBuffer<objConst> g_objConst : register(t2);

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
	result.position = mul(perspectiveMatrix, mul(viewMatrix, mul(objC.TransMatrix, g_VectorSign[vid])));
	return result;
}