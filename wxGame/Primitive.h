#pragma once
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
		{-1.f,1.f,1.f,1.f},		//far plane left up corner
		{-1.f,-1.f,1.f,1.f},	//far plane left down corner
		{1.f,1.f,1.f,1.f},		//far plane right up corner
		{1.f,-1.f,1.f,1.f},		//far plane right down corner
		{-1.f,1.f,-1.f,1.f},	//near plane left up corner
		{-1.f,-1.f,-1.f,1.f},	//near plane left down corner
		{1.f,1.f,-1.f,1.f},		//near plane right up corner
		{1.f,-1.f,-1.f,1.f},	//near plane right down corner
	};
}