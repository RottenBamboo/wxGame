cbuffer cbBlurWeight : register(b0)
{
	float4 blurWeights[3];
}

static const int gBlurRadius = 5;
Texture2D verticalMapInput : register(t0);
RWTexture2D<float4> verticalMapOutput : register(u0);

#define N 256
#define CacheSize (N + 2*gBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(1, N, 1)]
void BlurVerticalCS( uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float BlurWeightsArray[12] =
	{
		blurWeights[0].x, blurWeights[0].y, blurWeights[0].z, blurWeights[0].w,
		blurWeights[1].x, blurWeights[1].y, blurWeights[1].z, blurWeights[1].w,
		blurWeights[2].x, blurWeights[2].y, blurWeights[2].z, blurWeights[2].w,
	};
	if (groupThreadID.y < gBlurRadius)
	{
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[groupThreadID.y] = verticalMapInput[int2(dispatchThreadID.x, y)];
	}
	if (groupThreadID.y >= N - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + gBlurRadius, verticalMapInput.Length.y - 1);
		gCache[groupThreadID.y + 2 * gBlurRadius] = verticalMapInput[int2(dispatchThreadID.x, y)];
	}
	gCache[groupThreadID.y + gBlurRadius] = verticalMapInput[min(dispatchThreadID.xy, verticalMapInput.Length.xy - 1)];
	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0, 0, 0, 0);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;

		blurColor += BlurWeightsArray[i + gBlurRadius] * gCache[k];
	}
	verticalMapOutput[dispatchThreadID.xy] = blurColor;
}