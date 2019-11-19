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

struct Vertex
{
	float4 position;
	float2 uv;
	float3 Normal;
	float3 tangent;
};

struct BoundingBoxVertexIndex
{
	Vertex Vertex[8];
	int Index[16];
};

struct BoundingBox
{
	float3 Center;
	float3 Extents;
	BoundingBoxVertexIndex CornerPosition;
};

struct objConst
{
	matrix TransMatrix;
	BoundingBox Box;
};

StructuredBuffer<objConst> g_objConst : register(t2);