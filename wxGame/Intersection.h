#pragma once
#include "Mathmatic.h"
#include "wxGraphicD3D12.h"
#include "Primitive.h"

using namespace Mathmatic;
using namespace wxGame;

namespace wxGame
{
	struct Box
	{
	public:
		Vector3FT Min;
		Vector3FT Max;
		Box() :Min(0), Max(0) {};
	};

	struct BoundingBoxVertexIndex
	{
		std::vector<Vertex> Vertex;
		std::vector<int> Index;
		BoundingBoxVertexIndex()
		{
			Vertex.resize(8);
		}
	};
	struct BoundingBox
	{
		Vector3FT Center;
		Vector3FT Extents;
		BoundingBoxVertexIndex CornerPosition;
		BoundingBox(Vector3FT center, Vector3FT radius)
		{
			Center = center; Extents = radius;
		}
		BoundingBox() :Center(0), Extents(0) {}
	};

	struct BoundingSphere
	{
		Vector3FT Center;
		float Radius;
	};

	struct BoundingFrustum
	{
		Vector3FT Origin;
	};

	class BoundingGeometryMgr
	{
	public:
		void CompulateBoundingBox(BoundingBox& boundingBox, Vertex& vertex, size_t length)
		{
			Vertex* curr_vertex = &vertex;
			Box AABB;
			AABB.Min[0] = curr_vertex->position[0];
			AABB.Min[1] = curr_vertex->position[1];
			AABB.Min[2] = curr_vertex->position[2];
			AABB.Max = AABB.Min; 
			int i = 0;
			for ( ;i < length; i++)
			{
				AABB.Min[0] = MATH_MIN(curr_vertex[i].position[0], AABB.Min[0]);
				AABB.Min[1] = MATH_MIN(curr_vertex[i].position[1], AABB.Min[1]);
				AABB.Min[2] = MATH_MIN(curr_vertex[i].position[2], AABB.Min[2]);
				AABB.Max[0] = MATH_MAX(curr_vertex[i].position[0], AABB.Max[0]);
				AABB.Max[1] = MATH_MAX(curr_vertex[i].position[1], AABB.Max[1]);
				AABB.Max[2] = MATH_MAX(curr_vertex[i].position[2], AABB.Max[2]);
			}
			boundingBox.Center = AABB.Min + (AABB.Max - AABB.Min) / 2;
			boundingBox.Extents = (AABB.Max - AABB.Min) / 2;
			ComputeBoundingBoxCornerPosition(boundingBox);
		}

		void TransformAABB(BoundingBox& boundingBox, Matrix4X4FT matrix)
		{
			ComputeBoundingBoxCornerPosition(boundingBox);

			for (int i = 0; i != 8; i++)
			{
				VectorMultiMatrix(boundingBox.CornerPosition.Vertex[i].position, matrix);
			}
			CompulateBoundingBox(boundingBox, boundingBox.CornerPosition.Vertex[0], 8);
		}

		void ComputeBoundingBoxCornerPosition(BoundingBox& boundingBox)
		{
			boundingBox.CornerPosition.Vertex[0].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[0].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[0].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[0].position[3] = 1;

			boundingBox.CornerPosition.Vertex[1].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[1].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[1].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[1].position[3] = 1;

			boundingBox.CornerPosition.Vertex[2].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[2].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[2].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[2].position[3] = 1;

			boundingBox.CornerPosition.Vertex[3].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[3].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[3].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[3].position[3] = 1;

			boundingBox.CornerPosition.Vertex[4].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[4].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[4].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[4].position[3] = 1;

			boundingBox.CornerPosition.Vertex[5].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[5].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[5].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[5].position[3] = 1;

			boundingBox.CornerPosition.Vertex[6].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[6].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[6].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[6].position[3] = 1;

			boundingBox.CornerPosition.Vertex[7].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[7].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[7].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[7].position[3] = 1;

			boundingBox.CornerPosition.Index = {0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4};
		}

		void CompulateBoundingSphere(BoundingBox& boundingBox, Vertex& vertex, size_t length)
		{
			CompulateBoundingBox(boundingBox, vertex, length);
			boundingBox.Extents;
			m_boundingSphere.Center = boundingBox.Center;
			m_boundingSphere.Radius = sqrt(pow(boundingBox.Extents[0], 2) + pow(boundingBox.Extents[1], 2) + pow(boundingBox.Extents[2], 2));
		}
		Vertex* boundingBoxVertex;
		BoundingSphere m_boundingSphere;
	};
}