//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer cmatrix:register(b0)
{
	matrix rotatMatrix;
	matrix viewMatrix;
	matrix perspectiveMatrix;
}

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD)
{
	PSInput result;
	result.position = mul(perspectiveMatrix, mul(viewMatrix, mul(rotatMatrix, position)));
	result.uv.x = uv.x;
	result.uv.y = 1.f - uv.y;//v should be vertically reversed because the texture mapping is opposite direction when right hand coordinate transformed to left hand coordinate.

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return g_texture.Sample(g_sampler, input.uv);
}
