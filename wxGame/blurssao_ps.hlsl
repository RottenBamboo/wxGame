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
	matrix invProjMatrix;
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
	float DimensionWidth;
	float DimensionHeight;
	float g_horizontalBlur;
	float4 blurWeights[3];
}

static const int gBlurRadius = 5;
Texture2D g_ambientmap : register(t6);
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
	float viewZ = perspectiveMatrix[2][3] / (z_ndc - perspectiveMatrix[2][2]);//hlsl中举证是列主序，需要c++传递转置矩阵，现在暂使用行主序对应的下表位置
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

float4 PSMain(PSOutput input) : SV_Target
{
	float BlurWeightsArray[12] =
	{
		blurWeights[0].x, blurWeights[0].y, blurWeights[0].z, blurWeights[0].w,
		blurWeights[1].x, blurWeights[1].y, blurWeights[1].z, blurWeights[1].w,
		blurWeights[2].x, blurWeights[2].y, blurWeights[2].z, blurWeights[2].w,
	};

	float2 texOffset;
	if (g_horizontalBlur >= 1.0f)
	{
		texOffset = float2(float(1/DimensionWidth), 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, float(1/DimensionHeight));
	}

	// The center value always contributes to the sum.
	float4 color = BlurWeightsArray[gBlurRadius] * g_ambientmap.SampleLevel(g_PointClampSampler, input.uv, 0.0);
	float totalWeight = BlurWeightsArray[gBlurRadius];

	float3 centerNormal = g_normalmap.SampleLevel(g_PointClampSampler, input.uv, 0.0f).xyz;
	float  centerDepth = NdcDepthToViewDepth(
		g_depthMap.SampleLevel(g_depthMapSampler, input.uv, 0.0f).r);

	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		// We already added in the center weight.
		if (i == 0)
			continue;

		float2 tex = input.uv + i * texOffset;

		float3 neighborNormal = g_normalmap.SampleLevel(g_PointClampSampler, tex, 0.0f).xyz;
		float  neighborDepth = NdcDepthToViewDepth(
			g_depthMap.SampleLevel(g_depthMapSampler, tex, 0.0f).r);

		//
		// If the center value and neighbor values differ too much (either in 
		// normal or depth), then we assume we are sampling across a discontinuity.
		// We discard such samples from the blur.
		//

		if (dot(neighborNormal, centerNormal) >= 0.8f &&
			abs(neighborDepth - centerDepth) <= 0.2f)
		{
			float weight = BlurWeightsArray[i + gBlurRadius];

			// Add neighbor pixel to blur.
			color += weight * g_ambientmap.SampleLevel(
				g_PointClampSampler, tex, 0.0);

			totalWeight += weight;
		}
	}

	// Compensate for discarded samples by making total weights sum to 1.
	return color / totalWeight;
}