#include "Mesh.h"
#include "Mathmatic.h"
using namespace Mathmatic;

MeshInfo SimpleGeometryGenerator::GenerateCylinder(float bottomRadius, float topRadius, float height, unsigned int sliceCount, unsigned int stackCount)
{
	MeshInfo mesh;
	float stackHeight = height / stackCount;
	float radiusStep = (topRadius - bottomRadius) / stackCount;
	unsigned ringCount = stackCount + 1;
	float delta = 2.f * PI / sliceCount;
	Vertex vertex;
	for (int i = 0; i < ringCount; i++)
	{
		float y = -0.5f * height + i * stackHeight; //half sphere position under origin position
		float r = bottomRadius + radiusStep * i;
		for (int j = 0; j <= sliceCount; ++j)
		{
			float c = cosf(j * delta);
			float s = sinf(j * delta);
			vertex.position = { r * c, y, r * s, 1.f };
			vertex.uv[0] = (float)j / sliceCount;
			vertex.uv[1] = (float)i / stackCount;
			vertex.tangent = {-s, 0.f, c};
			float dr = bottomRadius - topRadius;
			Vector3FT bitangent = (dr * c, -height, dr * s);
			vertex.Normal = vectorNormalize(vectorCrossProduct(bitangent, vertex.tangent));
			mesh.vec_vertices.push_back(vertex);
		}
	}

	int ringVertexCount = sliceCount + 1;
	//indices each stack
	for (int i = 0; i < stackCount; i++)
	{
		for (int j = 0; j < sliceCount; j++)
		{
			mesh.vec_indices.push_back(i * ringVertexCount + j);
			mesh.vec_indices.push_back((i + 1) * ringVertexCount + j + 1);
			mesh.vec_indices.push_back(i * ringVertexCount + j + 1);

			mesh.vec_indices.push_back(i * ringVertexCount + j);
			mesh.vec_indices.push_back((i + 1) * ringVertexCount + j);
			mesh.vec_indices.push_back((i + 1) * ringVertexCount + j + 1);
		}
	}

	//generate top cap
	int capBeginIndex = (int)mesh.vec_vertices.size();
	for (int i = 0; i <= sliceCount; i++)
	{
		vertex.position[0] = topRadius * cosf(i * delta);
		vertex.position[1] = 0.5f * height; //top cap height
		vertex.position[2] = topRadius * sinf(i * delta);
		vertex.position[3] = 1.f;
		vertex.tangent = { 1.0f, 0.0f, 0.0f };
		vertex.Normal = { 0.0f, 1.0f, 0.0f };
		vertex.uv = { vertex.position[0] / height + 0.5f, vertex.position[2] / height + 0.5f };
		mesh.vec_vertices.push_back(vertex);
	}

	//cap center
	vertex.position = { 0.f, 0.5f * height, 0.f, 1.f };
	vertex.Normal = { 0.f, 1.f, 0.f };
	vertex.tangent = { 1.f, 0.f, 0.f };
	vertex.uv = { 0.5f, 0.5f };
	mesh.vec_vertices.push_back(vertex);

	//cap center index
	int capCenterIndex = (int)mesh.vec_vertices.size() - 1;

	for (int i = 0; i < sliceCount; i++)
	{
		mesh.vec_indices.push_back(capCenterIndex);
		mesh.vec_indices.push_back(capBeginIndex + i + 1);
		mesh.vec_indices.push_back(capBeginIndex + i);
	}

	//generate bottom cap
	capBeginIndex = (int)mesh.vec_vertices.size();
	for (int i = 0; i <= sliceCount; i++)
	{
		vertex.position[0] = bottomRadius * cosf(i * delta);
		vertex.position[1] = -0.5f * height; //top cap height
		vertex.position[2] = bottomRadius * sinf(i * delta);
		vertex.position[3] = 1.f;
		vertex.tangent = { 1.0f, 0.0f, 0.0f };
		vertex.Normal = { 0.0f, -1.0f, 0.0f };
		vertex.uv = { vertex.position[0] / height + 0.5f, vertex.position[2] / height + 0.5f };
		mesh.vec_vertices.push_back(vertex);
	}

	//cap center
	vertex.position = { 0.f, -0.5f * height, 0.f, 1.f };
	vertex.Normal = { 0.f, -1.f, 0.f };
	vertex.tangent = { 1.f, 0.f, 0.f };
	vertex.uv = { 0.5f, 0.5f };
	mesh.vec_vertices.push_back(vertex);

	//cap center index
	capCenterIndex = (int)mesh.vec_vertices.size() - 1;

	for (int i = 0; i < sliceCount; i++)
	{
		mesh.vec_indices.push_back(capCenterIndex);
		mesh.vec_indices.push_back(capBeginIndex + i);
		mesh.vec_indices.push_back(capBeginIndex + i + 1);
	}
	return mesh;
}

MeshInfo SimpleGeometryGenerator::GenerateSphere(float radius, unsigned int sliceCount, unsigned int stackCount)
{
	MeshInfo mesh;
	Vertex topVertex; topVertex.position = { 0.f, radius, 0.f, 1.f }; topVertex.Normal = { 0.f, 1.f, 0.f }; topVertex.tangent = { 1.f, 0.f, 0.f }; topVertex.uv = { 0.f,0.f };
	Vertex bottomVertex; topVertex.position = { 0.f, -radius, 0.f, 1.f }; topVertex.Normal = { 0.f, -1.f, 0.f }; topVertex.tangent = { 1.f, 0.f, 0.f }; topVertex.uv = { 0.f,1.f };
	
	mesh.vec_vertices.push_back(topVertex);
	float verticalStep = PI / stackCount;
	float horizentalStep = 2 * PI / sliceCount;

	//vertical step ring
	for (int i = 1; i <= stackCount - 1; i++)
	{
		float vertical = i * verticalStep;
		for (int j = 0; j <= sliceCount; j++)
		{
			float horizental = j * horizentalStep;
			Vertex vertex;
			vertex.position[0] = radius * sinf(vertical) * cosf(horizental);
			vertex.position[1] = radius * cosf(vertical);
			vertex.position[2] = radius * sinf(vertical) * sinf(horizental);
			vertex.position[3] = 1.f;
			vertex.position = vectorNormalize(vertex.position);
			vertex.tangent[0] = -radius * sinf(vertical) * sinf(horizental);
			vertex.tangent[1] = 0.f;
			vertex.tangent[2] = radius * sinf(vertical) * cosf(horizental);
			vertex.tangent = vectorNormalize(vertex.tangent);

			Vector4FT normal = vectorNormalize(vertex.position);
			vertex.Normal[0] = normal[0];
			vertex.Normal[1] = normal[1];
			vertex.Normal[2] = normal[2];

			vertex.uv[0] = horizental / (2 * PI);
			vertex.uv[1] = vertical / PI;

			mesh.vec_vertices.push_back(vertex);
		}
	}
	mesh.vec_vertices.push_back(bottomVertex);

	//top primitive index
	for (int i = 0; i <= sliceCount; i++)
	{
		mesh.vec_indices.push_back(0);
		mesh.vec_indices.push_back(i);
		mesh.vec_indices.push_back(i + 1);
	}
	//surrounding primitive
	int topIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 1; i < stackCount - 1; i++) // no top pole or bottom pole vertex
	{
		for (int j = 0; j < sliceCount; j++)
		{
			mesh.vec_indices.push_back(topIndex + i * ringVertexCount + j);
			mesh.vec_indices.push_back(topIndex + (i + 1) * ringVertexCount + j);
			mesh.vec_indices.push_back(topIndex + i * ringVertexCount + j + 1);

			mesh.vec_indices.push_back(topIndex + i * ringVertexCount + j);
			mesh.vec_indices.push_back(topIndex + (i + 1) * ringVertexCount + j);
			mesh.vec_indices.push_back(topIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}
	//bottom pole index
	int bottomIndex = mesh.vec_vertices.size() - 1;
	//bottom first primitive index
	int bottomPrimitiveIndex = bottomIndex - ringVertexCount;
	for (int i = 0; i < sliceCount; i++)
	{
		mesh.vec_indices.push_back(bottomPrimitiveIndex + i);
		mesh.vec_indices.push_back(bottomIndex);
		mesh.vec_indices.push_back(bottomPrimitiveIndex + i + 1);
	}
	return mesh;
}
