#pragma once
#include "Primitive.h"

using namespace Mathmatic;
using namespace wxGame;

namespace wxGame
{
	struct Box
	{
	public:
		Vector4FT Min;
		Vector4FT Max;
		Box() :Min(0), Max(0) {};
	};

	struct BoundingBoxVertexIndex
	{
		Vertex Vertex[24];
	};
	struct BoundingBox
	{
		Vector4FT Center;
		Vector4FT Extents;
		BoundingBoxVertexIndex CornerPosition;
		BoundingBox(Vector4FT center, Vector4FT radius)
		{
			Center = center; Extents = radius;
		}
		BoundingBox() :Center(0), Extents(0) {}
	};

	struct BoundingSphere
	{
		Vector4FT Center;
		float Radius;
	};

	struct BoundingFrustum
	{
		Vector4FT Origin;
	};

	class BoundingGeometryMgr
	{
	public:
		void CalculateBoundingBoxByVertex(BoundingBox& boundingBox, Vertex& vertex, size_t length);

		void  CalculateBoundingBoxCornerPositionByVertex(BoundingBox& boundingBox, Vertex& vertex);

		void TransformAABB(BoundingBox& boundingBox, Matrix4X4FT& transformMatrix);

		void TransformBoundingBoxCornerPosition(BoundingBox& boundingBox, Matrix4X4FT& Matrix);

		void  CalculateBoundingBoxCornerPosition(BoundingBox& boundingBox);

		void  CalculateBoundingSphere(BoundingBox& boundingBox, Vertex& vertex, size_t length);

		BoundingSphere m_boundingSphere;
	};
}