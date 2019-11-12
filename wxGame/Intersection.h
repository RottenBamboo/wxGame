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

	class BoundingBoxMgr
	{
	public:
		void CompulateBoundingBox(Vertex& vertex, size_t length)
		{
			Vertex* curr_vertex = &vertex;
			Box AABB;
			AABB.Min.element[0] = curr_vertex->position.element[0];
			AABB.Min.element[1] = curr_vertex->position.element[1];
			AABB.Min.element[2] = curr_vertex->position.element[2];
			AABB.Max = AABB.Min; 
			int i = 0;
			for ( ;i < length; i++)
			{
				AABB.Min.element[0] = MATH_MIN(curr_vertex[i].position.element[0], AABB.Min.element[0]);
				AABB.Min.element[1] = MATH_MIN(curr_vertex[i].position.element[1], AABB.Min.element[1]);
				AABB.Min.element[2] = MATH_MIN(curr_vertex[i].position.element[2], AABB.Min.element[2]);
				AABB.Max.element[0] = MATH_MAX(curr_vertex[i].position.element[0], AABB.Max.element[0]);
				AABB.Max.element[1] = MATH_MAX(curr_vertex[i].position.element[1], AABB.Max.element[1]);
				AABB.Max.element[2] = MATH_MAX(curr_vertex[i].position.element[2], AABB.Max.element[2]);
			}
			m_boundingBox.Center = AABB.Min + (AABB.Max - AABB.Min) / 2;
			m_boundingBox.Extents = (AABB.Max - AABB.Min) / 2;
		}
		BoundingBox m_boundingBox;
	};
}