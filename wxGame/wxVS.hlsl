struct VS_INPUT
{
	float4 Pos : POSITION;	// Upper-left position in screen pixel coordinates
	float4 Clr : COLOR;		// X, Y, Width, Height in texel space
	float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;	// Upper-left and lower-right coordinates in clip space
	float4 Clr : COLOR;		// Upper-left and lower-right normalized UVs
	float2 TexCoord : TEXCOORD0;
};

cbuffer cmatrix:register(b0)
{
	matrix rotatMatrix;
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix wvpMatrix;
}

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT result;
	result.Pos = input.Pos;
	//matrix temp = mul(rotatMatrix, viewMatrix);
	//result.Pos = mul(input.Pos, rotatMatrix);
	//result.Pos = mul(input.Pos, viewMatrix);
	//result.Pos = mul(input.Pos, perspectiveMatrix);
	result.Pos = mul(input.Pos, wvpMatrix);
	result.Clr = input.Clr;
	result.TexCoord = input.TexCoord;
	return result;
}