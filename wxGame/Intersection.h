#pragma once
#include "Mathmatic.h"
using namespace Mathmatic;
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