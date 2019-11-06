#pragma once
#include "Mathmatic.h"
#include "wxGraphicD3D12.h"

using namespace Mathmatic;

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
		bool CompulateBoundingBox(wxGraphicD3D12::Vertex& vertex, size_t length)
		{
			for (int i = 0; i < length; i++)
			{
				
			}
		}
	};


	class BoundingBoxMgr
	{
	private:
		BoundingBox m_boundingBox;
	};