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
			vertex.uv[1] = 1.f - (float)i / stackCount;
			vertex.tangent = {-s, 0.f, c};
			float dr = bottomRadius - topRadius;
			Vector3FT bitangent = (dr * c, -height, dr * s);
			vertex.Normal = vectorNormalize(vectorCrossProduct(vertex.tangent, bitangent));
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
			mesh.vec_indices.push_back(i * ringVertexCount + j + 1);
			mesh.vec_indices.push_back((i + 1) * ringVertexCount + j + 1);

			mesh.vec_indices.push_back(i * ringVertexCount + j);
			mesh.vec_indices.push_back((i + 1) * ringVertexCount + j + 1);
			mesh.vec_indices.push_back((i + 1) * ringVertexCount + j);
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
		mesh.vec_indices.push_back(capBeginIndex + i);
		mesh.vec_indices.push_back(capBeginIndex + i + 1);
	}

	//generate bottom cap
	capBeginIndex = (int)mesh.vec_vertices.size();
	for (int i = 0; i <= sliceCount; i++)
	{
		vertex.position[0] = topRadius * cosf(i * delta);
		vertex.position[1] = -0.5f * height; //top cap height
		vertex.position[2] = topRadius * sinf(i * delta);
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
		mesh.vec_indices.push_back(capBeginIndex + i);
		mesh.vec_indices.push_back(capCenterIndex);
		mesh.vec_indices.push_back(capBeginIndex + i + 1);
	}
	return mesh;
}
