#pragma once
#include "Mathmatic.h"
using namespace Mathmatic;

namespace wxGame
{
	struct Vertex
	{
		Vector4FT position;
		Vector2FT uv;
		Vector3FT Normal;
		Vector3FT tangent;
	};

	struct ViewFrustum
	{
		Plane4FT plane_top;
		Plane4FT plane_bottom;
		Plane4FT plane_left;
		Plane4FT plane_right;
		Plane4FT plane_far;
		Plane4FT plane_near;
	};

	static Vector4FT FrustumCornerVertex[8] =
	{
		{-1.f,1.f,1.f,1.f},		//far plane left top corner
		{-1.f,-1.f,1.f,1.f},	//far plane left bottom corner
		{1.f,1.f,1.f,1.f},		//far plane right top corner
		{1.f,-1.f,1.f,1.f},		//far plane right bottom corner
		{-1.f,1.f,-1.f,1.f},	//near plane left top corner
		{-1.f,-1.f,-1.f,1.f},	//near plane left bottom corner
		{1.f,1.f,-1.f,1.f},		//near plane right top corner
		{1.f,-1.f,-1.f,1.f},	//near plane right bottom corner
	};
}