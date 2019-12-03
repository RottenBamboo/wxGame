#pragma once
#include "Intersection.h"

void BoundingGeometryMgr::CalculateBoundingBoxByVertex(BoundingBox& boundingBox, Vertex& vertex, size_t length)
{
	Vertex* curr_vertex = &vertex;
	Box AABB;
	AABB.Min[0] = curr_vertex->position[0];
	AABB.Min[1] = curr_vertex->position[1];
	AABB.Min[2] = curr_vertex->position[2];
	AABB.Max = AABB.Min;
	int i = 0;
	for (; i < length; i++)
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
	CalculateBoundingBoxCornerPosition(boundingBox);
}

void  BoundingGeometryMgr::CalculateBoundingBoxCornerPositionByVertex(BoundingBox& boundingBox, Vertex& vertex)
{
	Vertex* curr_vertex = &vertex;
	Box AABB;
	AABB.Min[0] = curr_vertex->position[0];
	AABB.Min[1] = curr_vertex->position[1];
	AABB.Min[2] = curr_vertex->position[2];
	AABB.Max = AABB.Min;
	int i = 0;
	for (; i < 24; i++)
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
	CalculateBoundingBoxCornerPosition(box);
	boundingBox.CornerPosition = box.CornerPosition;
}

void BoundingGeometryMgr::TransformAABB(BoundingBox& boundingBox, Matrix4X4FT& transformMatrix)
{
	//calculate original AABB corner position;
	CalculateBoundingBoxCornerPosition(boundingBox);
	//calculate current AABB corner position after transformation and rotation;
	TransformBoundingBoxCornerPosition(boundingBox, transformMatrix);
	//calculate inaccurate AABB  by current corner position;
	CalculateBoundingBoxCornerPositionByVertex(boundingBox, boundingBox.CornerPosition.Vertex[0]);
}

void BoundingGeometryMgr::TransformBoundingBoxCornerPosition(BoundingBox& boundingBox, Matrix4X4FT& Matrix)
{
	int i = 0;
	for (; i < 24; i++)
	{
		VectorMultiMatrix(boundingBox.CornerPosition.Vertex[i].position, Matrix);
	}
}

void BoundingGeometryMgr::CalculateBoundingBoxCornerPosition(BoundingBox& boundingBox)
{
	Vertex seqVertex[8];
	seqVertex[0].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
	seqVertex[0].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
	seqVertex[0].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
	seqVertex[0].position[3] = 1;

	seqVertex[1].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
	seqVertex[1].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
	seqVertex[1].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
	seqVertex[1].position[3] = 1;

	seqVertex[2].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
	seqVertex[2].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
	seqVertex[2].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
	seqVertex[2].position[3] = 1;

	seqVertex[3].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
	seqVertex[3].position[1] = boundingBox.Center[1] - boundingBox.Extents[1];
	seqVertex[3].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
	seqVertex[3].position[3] = 1;

	seqVertex[4].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
	seqVertex[4].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
	seqVertex[4].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
	seqVertex[4].position[3] = 1;

	seqVertex[5].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
	seqVertex[5].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
	seqVertex[5].position[2] = boundingBox.Center[2] - boundingBox.Extents[2];
	seqVertex[5].position[3] = 1;

	seqVertex[6].position[0] = boundingBox.Center[0] + boundingBox.Extents[0];
	seqVertex[6].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
	seqVertex[6].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
	seqVertex[6].position[3] = 1;

	seqVertex[7].position[0] = boundingBox.Center[0] - boundingBox.Extents[0];
	seqVertex[7].position[1] = boundingBox.Center[1] + boundingBox.Extents[1];
	seqVertex[7].position[2] = boundingBox.Center[2] + boundingBox.Extents[2];
	seqVertex[7].position[3] = 1;

	//0
	boundingBox.CornerPosition.Vertex[0] = seqVertex[0];
	//1
	boundingBox.CornerPosition.Vertex[1] = seqVertex[1];

	//1
	boundingBox.CornerPosition.Vertex[2] = seqVertex[1];
	//2
	boundingBox.CornerPosition.Vertex[3] = seqVertex[2];

	//2
	boundingBox.CornerPosition.Vertex[4] = seqVertex[2];
	//3
	boundingBox.CornerPosition.Vertex[5] = seqVertex[3];

	//3
	boundingBox.CornerPosition.Vertex[6] = seqVertex[3];
	//0
	boundingBox.CornerPosition.Vertex[7] = seqVertex[0];

	//4
	boundingBox.CornerPosition.Vertex[8] = seqVertex[4];
	//5
	boundingBox.CornerPosition.Vertex[9] = seqVertex[5];

	//5
	boundingBox.CornerPosition.Vertex[10] = seqVertex[5];
	//6
	boundingBox.CornerPosition.Vertex[11] = seqVertex[6];

	//6
	boundingBox.CornerPosition.Vertex[12] = seqVertex[6];
	//7
	boundingBox.CornerPosition.Vertex[13] = seqVertex[7];

	//7
	boundingBox.CornerPosition.Vertex[14] = seqVertex[7];
	//4
	boundingBox.CornerPosition.Vertex[15] = seqVertex[4];

	//0
	boundingBox.CornerPosition.Vertex[16] = seqVertex[0];
	//4
	boundingBox.CornerPosition.Vertex[17] = seqVertex[4];

	//1
	boundingBox.CornerPosition.Vertex[18] = seqVertex[1];
	//5
	boundingBox.CornerPosition.Vertex[19] = seqVertex[5];

	//2
	boundingBox.CornerPosition.Vertex[20] = seqVertex[2];
	//6
	boundingBox.CornerPosition.Vertex[21] = seqVertex[6];

	//3
	boundingBox.CornerPosition.Vertex[22] = seqVertex[3];
	//7
	boundingBox.CornerPosition.Vertex[23] = seqVertex[7];
}

void  BoundingGeometryMgr::CalculateBoundingSphere(BoundingBox& boundingBox, Vertex& vertex, size_t length)
{
	CalculateBoundingBoxByVertex(boundingBox, vertex, length);
	boundingBox.Extents;
	m_boundingSphere.Center = boundingBox.Center;
	m_boundingSphere.Radius = sqrt(pow(boundingBox.Extents[0], 2) + pow(boundingBox.Extents[1], 2) + pow(boundingBox.Extents[2], 2));
}