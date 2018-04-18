struct PS_INPUT
{
	float4 Pos : SV_POSITION;	// Upper-left and lower-right coordinates in clip space
	float4 Clr : COLOR;		// Upper-left and lower-right normalized UVs'
	float2 TexCoord : TEXCOORD0;
};
Texture2D			colorMap : register(t0);
SamplerState		texSampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 result;

	//float4 sss = colorMap.Load(int3(input.TexCoord[0], input.TexCoord[1], 1));
	//result = colorMap.Sample(texSampler, input.TexCoord);
	result.xyzw = input.Clr.xyzw;
	return result;
}