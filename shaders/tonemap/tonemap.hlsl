Texture2D hdrMap : register(t0);
SamplerState samplerState : register(s0);

struct vsInput
{
	uint index : SV_VertexID;
};

struct psInput
{
	float4 position : SV_POSITION;
	float2 tcoord: TEXCOORD;
};


psInput vsmain(vsInput input)
{
	psInput output;
	
	float2 texcoord = float2(input.index & 1, input.index >> 1);
	output.position = float4((texcoord.x - 0.5f) * 2, -(texcoord.y - 0.5f) * 2, 1, 1);
	output.tcoord = output.position.xy;
	return output;
}

float3 toneMap(float3 color)
{
	//https://www.shadertoy.com/view/lslGzl
	//float exposure = 1.1f;
	//hdrColor *= exposure / (1.0f + hdrColor / exposure);
	//return hdrColor;

	//float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
	//float toneMappedLuma = luma / (1. + luma);
	//color *= toneMappedLuma / luma;

	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	float exposure = 1.1;
	color *= exposure;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;

	return color;
}

float4 psmain(psInput input) : SV_Target
{
	float2 tcoord = input.tcoord*0.5f + 0.5f;
	tcoord.y = 1.0f - tcoord.y;
	
	float3 hdrColor = hdrMap.Sample(samplerState, tcoord).xyz;
	float3 ldrColor = toneMap(hdrColor);

	float3 gammaCorrectedColor = pow(ldrColor, 1.0f / 2.2f);
	return float4(gammaCorrectedColor, 0.0f);
}