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
	};

	struct BoundingBox
	{
		Vector3FT Center;
		Vector3FT Radius;
		BoundingBox(Vector3FT center, Vector3FT radius)
		{
			Center = center; Radius = radius;
		}
		BoundingBox() :Center(), Radius() {}
	};

	class BoundingBoxMgr
	{
	public:
		void CompulateBoundingBox(Vertex& vertex, size_t length)
		{
			Box AABB;
			for (int i = 0; i < length; i++)
			{
				AABB.Min.element[0] = MATH_MIN(vertex.position.element[0], AABB.Min.element[0]);
				AABB.Min.element[1] = MATH_MIN(vertex.position.element[1], AABB.Min.element[1]);
				AABB.Min.element[2] = MATH_MIN(vertex.position.element[2], AABB.Min.element[2]);
				AABB.Max.element[0] = MATH_MAX(vertex.position.element[0], AABB.Max.element[0]);
				AABB.Max.element[1] = MATH_MAX(vertex.position.element[1], AABB.Max.element[1]);
				AABB.Max.element[2] = MATH_MAX(vertex.position.element[2], AABB.Max.element[2]);
			}
			m_boundingBox.Center = AABB.Min + (AABB.Max - AABB.Min) / 2;
			m_boundingBox.Radius = (AABB.Max - AABB.Min) / 2;
		}
	private:
		BoundingBox m_boundingBox;
	};
}