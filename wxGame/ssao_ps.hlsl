struct PSOutput
{
	float4 position : SV_POSITION;
	float4 shadowPosition : POSITION0;
	float4 positionH : POSITION1;
	float4 PositionWorld : POSITION2;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
};

cbuffer cmatrix:register(b1)
{
	matrix viewMatrix;
	matrix perspectiveMatrix;
	matrix rotatMatrix;
	matrix shadowTransform;
	matrix lightOthgraphicMatrix;
	matrix lightViewMatrix1;
	matrix lightTransformNDC;
	matrix invViewMatrix;
	float4 cameraPos;
	float4 viewPos;
}

cbuffer ssaoMatrix:register(b2)
{
	float4 g_offsetVectors[14];

	float g_occlusionRadius;
	float g_occlusionFadeStart;
	float g_occlusionFadeEnd;
	float g_surfaceEpsilon;
}

Texture2D g_texture : register(t0);
Texture2D g_normalmap : register(t3);
Texture2D g_depthMap : register(t4);//g_shadowMap
Texture2D g_randomVecorMap : register(t5);
SamplerState g_sampler : register(s0);
SamplerState g_depthMapSampler : register(s3);
SamplerState g_PointClampSampler : register(s4);
SamplerState g_linearClampSampler : register(s5);
SamplerState g_linearWrapSampler : register(s6);
static const int g_SampleCount = 14;
float NdcDepthToViewDepth(float z_ndc)
{
	// z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
	float viewZ = viewMatrix[3][2] / (z_ndc - viewMatrix[2][2]);
	return viewZ;
}

// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//

	float occlusion = 0.0f;
	if (distZ > g_surfaceEpsilon)
	{
		float fadeLength = g_occlusionFadeEnd - g_occlusionFadeStart;

		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = saturate((g_occlusionFadeEnd - distZ) / fadeLength);
	}

	return occlusion;
}

float PSMain(PSOutput input) : SV_Target
{
	float3 n = normalize(g_normalmap.SampleLevel(g_PointClampSampler, input.uv, 0.0f).xyz);
	float pz = g_depthMap.SampleLevel(g_depthMapSampler, input.uv, 0.0f).r;
	pz = NdcDepthToViewDepth(pz);
	float3 p = (pz / input.position.z)*input.position;
	float3 randomVector = 2.f * g_randomVecorMap.SampleLevel(g_linearWrapSampler, 4.f * input.uv, 0.f).rgb - 1.0f;
	float occlusionSum = 0.0f;

	for (int i = 0; i < g_SampleCount; i++)
	{
		float3 offset = reflect(g_offsetVectors[i].xyz, randomVector);
		float flip = sign(dot(offset, n));
		float3 q = p + flip * g_occlusionRadius * offset;
		float4 projQ = mul(lightTransformNDC, mul(perspectiveMatrix, float4(q, 1.0f)));
		projQ /= projQ.w;
		float rz = g_depthMap.SampleLevel(g_depthMapSampler, projQ.xy, 0.0f).r;
		rz = NdcDepthToViewDepth(rz);
		float3 r = (rz / q.z) * q;
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);
		occlusionSum += occlusion;
	}
	occlusionSum /= g_SampleCount;
	float access = 1.0f - occlusionSum;
	float result = saturate(pow(access, 6.0f));
	float rrr = { 1.0f };
	return rrr;
}