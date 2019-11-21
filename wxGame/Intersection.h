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
		Vector4FT Min;
		Vector4FT Max;
		Box() :Min(0), Max(0) {};
	};

	struct BoundingBoxVertexIndex
	{
		Vertex Vertex[16];
		int Index[16];
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

		void ComputeBoundingBoxCornerPositionByVertex(BoundingBox& boundingBox, Vertex& vertex)
		{
			Vertex* curr_vertex = &vertex;
			Box AABB;
			AABB.Min[0] = curr_vertex->position[0];
			AABB.Min[1] = curr_vertex->position[1];
			AABB.Min[2] = curr_vertex->position[2];
			AABB.Max = AABB.Min;
			int i = 0;
			for (; i < 16; i++)
			{
				AABB.Min[0] = MATH_MIN(curr_vertex[i].position[0], AABB.Min[0]);
				AABB.Min[1] = MATH_MIN(curr_vertex[i].position[1], AABB.Min[1]);
				AABB.Min[2] = MATH_MIN(curr_vertex[i].position[2], AABB.Min[2]);
				AABB.Max[0] = MATH_MAX(curr_vertex[i].position[0], AABB.Max[0]);
				AABB.Max[1] = MATH_MAX(curr_vertex[i].position[1], AABB.Max[1]);
				AABB.Max[2] = MATH_MAX(curr_vertex[i].position[2], AABB.Max[2]);
			}
			BoundingBox box;
			box.Center = AABB.Min + (AABB.Max - AABB.Min) / 2;
			box.Extents = (AABB.Max - AABB.Min) / 2;
			ComputeBoundingBoxCornerPosition(box);
			boundingBox.CornerPosition = box.CornerPosition;
		}

		void TransformAABB(BoundingBox& boundingBox, Matrix4X4FT& matrix)
		{
			TransformBoundingBoxCornerPosition(boundingBox, matrix);

			//ComputeBoundingBoxCornerPosition(boundingBox);
		}

		void TransformBoundingBoxCornerPosition(BoundingBox& boundingBox, Matrix4X4FT& matrix)
		{
			int i = 0;
			for (; i < 16; i++)
			{
				VectorMultiMatrix(boundingBox.CornerPosition.Vertex[i].position, matrix);
			}
		}

		void ComputeBoundingBoxCornerPosition(BoundingBox& boundingBox)
		{//0
			boundingBox.CornerPosition.Vertex[0].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[0].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[0].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[0].position[3] = 1;
		//1
			boundingBox.CornerPosition.Vertex[1].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[1].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[1].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[1].position[3] = 1;

		//1
			boundingBox.CornerPosition.Vertex[2].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[2].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[2].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[2].position[3] = 1;
		//2
			boundingBox.CornerPosition.Vertex[3].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[3].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[3].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[3].position[3] = 1;

		//2
			boundingBox.CornerPosition.Vertex[4].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[4].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[4].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[4].position[3] = 1;
		//3
			boundingBox.CornerPosition.Vertex[5].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[5].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[5].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[5].position[3] = 1;
		//3
			boundingBox.CornerPosition.Vertex[6].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[6].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[6].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[6].position[3] = 1;
		//0
			boundingBox.CornerPosition.Vertex[7].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[7].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[7].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[7].position[3] = 1;

		//4
			boundingBox.CornerPosition.Vertex[8].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[8].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[8].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[8].position[3] = 1;
		//5
			boundingBox.CornerPosition.Vertex[9].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[9].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[9].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[9].position[3] = 1;

		//5
			boundingBox.CornerPosition.Vertex[10].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[10].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[10].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[10].position[3] = 1;
		//6
			boundingBox.CornerPosition.Vertex[11].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[11].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[11].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[11].position[3] = 1;

		//6
			boundingBox.CornerPosition.Vertex[12].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[12].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[12].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[12].position[3] = 1;
		//7
			boundingBox.CornerPosition.Vertex[13].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[13].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[13].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[13].position[3] = 1;

		//7
			boundingBox.CornerPosition.Vertex[14].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[14].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[14].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[14].position[3] = 1;
		//4
			boundingBox.CornerPosition.Vertex[15].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
			boundingBox.CornerPosition.Vertex[15].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
			boundingBox.CornerPosition.Vertex[15].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
			boundingBox.CornerPosition.Vertex[15].position[3] = 1;

			int indexs[16] = { 0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4 };
			for (int i = 0; i != 16; i++)
			{
				boundingBox.CornerPosition.Index[i] = indexs[i];
			}
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