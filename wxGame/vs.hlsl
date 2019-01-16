cbuffer cmatrix:register(b2)
{
	matrix linearTransMatrix;
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix rotatMatrix;
	float4 cameraPos;
	float4 viewPos;
}

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : TEXCOORD3;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD, float3 normal : NORMAL)
{
	PSInput result;
	result.position = mul(perspectiveMatrix, mul(viewMatrix, mul(rotatMatrix,(mul(linearTransMatrix, position)))));
	result.uv.x = uv.x;
	result.uv.y = 1.f - uv.y;//v should be vertically reversed because the texture mapping is opposite direction when right hand coordinate transformed to left hand coordinate.
	result.normal = mul(rotatMatrix,mul(linearTransMatrix, normal));

	return result;
}