struct VSAttributes
{
	float3 position : POSITION;
	float2 texcoord0 : TEXCOORD;
	float3 normal : NORMAL;
	float tangent : TANGENT;
	float bitangent : BITANGENT;
};

struct VSOutput
{

};

float4 main( VSInput vsInput )
{
	VSAttributes vsOutput;

	return pos;
}