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

	struct BoundingBox
	{
		Vector3FT Center;
		Vector3FT Extents;
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

	class BoundingBoxMgr
	{
	public:
		void CompulateBoundingBox(Vertex& vertex, size_t length)
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
			m_boundingBox.Center = AABB.Min + (AABB.Max - AABB.Min) / 2;
			m_boundingBox.Extents = (AABB.Max - AABB.Min) / 2;
		}

		void TransformAABB(Matrix4X4FT matrix)
		{
			Vertex vertex[8];
			for (int i = 0; i != 8; i++)
			{
				vertex[0].position[0] = m_boundingBox.Center[0] - m_boundingBox.Extents[0];
				vertex[0].position[1] = m_boundingBox.Center[1] - m_boundingBox.Extents[1];
				vertex[0].position[2] = m_boundingBox.Center[2] - m_boundingBox.Extents[2];
				vertex[0].position[3] = 1;

				vertex[1].position[0] = m_boundingBox.Center[0] + m_boundingBox.Extents[0];
				vertex[1].position[1] = m_boundingBox.Center[1] - m_boundingBox.Extents[1];
				vertex[1].position[2] = m_boundingBox.Center[2] - m_boundingBox.Extents[2];
				vertex[1].position[3] = 1;

				vertex[2].position[0] = m_boundingBox.Center[0] + m_boundingBox.Extents[0];
				vertex[2].position[1] = m_boundingBox.Center[1] - m_boundingBox.Extents[1];
				vertex[2].position[2] = m_boundingBox.Center[2] + m_boundingBox.Extents[2];
				vertex[2].position[3] = 1;

				vertex[3].position[0] = m_boundingBox.Center[0] - m_boundingBox.Extents[0];
				vertex[3].position[1] = m_boundingBox.Center[1] - m_boundingBox.Extents[1];
				vertex[3].position[2] = m_boundingBox.Center[2] + m_boundingBox.Extents[2];
				vertex[3].position[3] = 1;

				vertex[4].position[0] = m_boundingBox.Center[0] - m_boundingBox.Extents[0];
				vertex[4].position[1] = m_boundingBox.Center[1] + m_boundingBox.Extents[1];
				vertex[4].position[2] = m_boundingBox.Center[2] - m_boundingBox.Extents[2];
				vertex[4].position[3] = 1;

				vertex[5].position[0] = m_boundingBox.Center[0] + m_boundingBox.Extents[0];
				vertex[5].position[1] = m_boundingBox.Center[1] + m_boundingBox.Extents[1];
				vertex[5].position[2] = m_boundingBox.Center[2] - m_boundingBox.Extents[2];
				vertex[5].position[3] = 1;

				vertex[6].position[0] = m_boundingBox.Center[0] + m_boundingBox.Extents[0];
				vertex[6].position[1] = m_boundingBox.Center[1] + m_boundingBox.Extents[1];
				vertex[6].position[2] = m_boundingBox.Center[2] + m_boundingBox.Extents[2];
				vertex[6].position[3] = 1;

				vertex[7].position[0] = m_boundingBox.Center[0] - m_boundingBox.Extents[0];
				vertex[7].position[1] = m_boundingBox.Center[1] + m_boundingBox.Extents[1];
				vertex[7].position[2] = m_boundingBox.Center[2] + m_boundingBox.Extents[2];
				vertex[7].position[3] = 1;
			}
			
			for (int i = 0; i != 8; i++)
			{
				VectorMultiMatrix(vertex[i].position, matrix);
			}
			CompulateBoundingBox(vertex[0], 8);
		}

		void CompulateBoundingSphere(Vertex& vertex, size_t length)
		{
			CompulateBoundingBox(vertex, length);
			m_boundingBox.Extents;
			m_boundingSphere.Center = m_boundingBox.Center;
			m_boundingSphere.Radius = sqrt(pow(m_boundingBox.Extents[0], 2) + pow(m_boundingBox.Extents[1], 2) + pow(m_boundingBox.Extents[2], 2));
		}
		BoundingBox m_boundingBox;
		BoundingSphere m_boundingSphere;
	};
}