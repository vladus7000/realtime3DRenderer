#include "common.hlsl"

RWTexture2D<float4> outTexture : register(u0);

[numthreads(20, 20, 1)]
void csMain(uint3 globalIdx : SV_DispatchThreadID,
            uint3 localIdx : SV_GroupThreadID,
            uint3 groupIdx : SV_GroupID)
{
    float2 tcoord = globalIdx.xy;

	float3 hdrColor = float3(0, 0, 0);
	
	hdrColor = litPixel(tcoord / float2(800, 600));

	outTexture[tcoord] = float4(hdrColor, 1.0f);
}