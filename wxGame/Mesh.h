#pragma once
#include "stdafx.h"
#include "Primitive.h"

using namespace wxGame;

namespace wxGame {
	typedef enum _PrimitiveType {
		kPrimitiveTypeNone = 0x00000000,
		kPrimitiveTypePointLIst = 0x00000001,
		kPrimitiveTypeLineLIst = 0x00000002,
		kPrimitiveTypeLineStrip = 0x00000003,
		kPrimitiveTypeTriList = 0x00000004,
		kPrimitiveTypeTriFan = 0x000000005,
		kPrimitiveTypeTriStrip = 0x00000006,
		kPrimitiveTypePatch = 0x00000009,
		kPrimitiveTypeLineListAdjacency = 0x0000000a,
		kPrimitiveTypeLineStripAdjacency = 0x0000000b,
		kPrimitiveTypeTriListAdjacency = 0x0000000c,
		kPrimitiveTypeTriStripAdjacency = 0x0000000d,
		kPrimitiveTypeRectList = 0x00000011,
		kPrimitiveTypeLineLoop = 0x00000012,
		kPrimitiveTypeQuadList = 0x00000013,
		kPrimitiveTypeQuadStrip = 0x00000014,
		kPrimitiveTypePolygon = 0x00000015,
	} PrimitiveType;

	struct MeshInfo
	{
		std::vector<Vertex> vec_vertices;
		std::vector<unsigned int> vec_indices;
	};

	class SimpleGeometryGenerator
	{
	public:
		MeshInfo GenerateCylinder(float bottomRadius, float topRadius, float height, unsigned int sliceCount, unsigned int stackCount);
	};
}
