cbuffer cbBlurWeight : register(b0)
{
	float4 blurWeights[3];
}

static const int gBlurRadius = 5;
Texture2D horizentalMapInput : register(t0);
RWTexture2D<float4> horizentalMapOutput : register(u0);

#define N 256
#define CacheSize (N + 2*gBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(N , 1, 1)]
void BlurHortizentalCS( uint3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float BlurWeightsArray[12] =
	{
		blurWeights[0].x, blurWeights[0].y, blurWeights[0].z, blurWeights[0].w,
		blurWeights[1].x, blurWeights[1].y, blurWeights[1].z, blurWeights[1].w,
		blurWeights[2].x, blurWeights[2].y, blurWeights[2].z, blurWeights[2].w,
	};
	//groupThreadID is thread id in a group.
	if (groupThreadID.x < gBlurRadius)
	{
		//dispatchThreadID is global thread id
		//dispatchThreadID.x - gBlurRadius clamp left bound sample to 0.
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = horizentalMapInput[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= N - gBlurRadius)
	{
		int x = min(dispatchThreadID.x + gBlurRadius, horizentalMapInput.Length.x - 1);
		gCache[groupThreadID.x + 2 * gBlurRadius] = horizentalMapInput[int2(x, dispatchThreadID.y)];
	}
	gCache[groupThreadID.x + gBlurRadius] = horizentalMapInput[min(dispatchThreadID.xy, horizentalMapInput.Length.xy - 1)];
	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0, 0, 0, 0);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;

		blurColor += BlurWeightsArray[i + gBlurRadius] * gCache[k];
	}
	horizentalMapOutput[dispatchThreadID.xy] = blurColor;
}