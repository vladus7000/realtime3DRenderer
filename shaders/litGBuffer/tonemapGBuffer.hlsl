cbuffer globals
{
	float3 ambientLight;
	float4 lightPosition_type;
	float3 lightDirection;
	float3 lightIntensity;
	float4x4 sunViewProjection;
	float3 camPos;
}

Texture2D hdrMap : register(t0);
Texture2D diffuseMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D posMap : register(t3);
TextureCube	g_EnvironmentTexture : register(t4);
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

float3 toneMap(float3 hdrColor)
{
	//https://www.shadertoy.com/view/lslGzl
	float exposure = 1.1f;
	hdrColor *= exposure / (1.0f + hdrColor / exposure);
	return hdrColor;
}

float3 finalColor(float3 light, float2 tcoord)
{
	float3 diffuseColor = diffuseMap.Sample(samplerState, tcoord).xyz;
	float3 normal = normalMap.Sample(samplerState, tcoord).xyz;
	float3 posPixel = posMap.Sample(samplerState, tcoord).xyz;
	//float3 refl = reflect(normalize(camPos.xyz - posPixel), normal);
	float3 fromeCubeMap = g_EnvironmentTexture.Sample(samplerState, normal).xyz;
	float intensity = dot(float3(0.2126f, 0.7152f, 0.0722f), light); //float3(0.2126f, 0.7152f, 0.0722f)

	return lerp(fromeCubeMap * diffuseColor, diffuseColor * light, saturate(intensity));
}

float4 psmain(psInput input) : SV_Target
{
	float2 tcoord = input.tcoord*0.5f + 0.5f;
	tcoord.y = 1.0f - tcoord.y;
	
	float3 hdrColor = hdrMap.Sample(samplerState, tcoord).xyz;
	float3 colored = finalColor(hdrColor, tcoord);
	float3 ldrColor = toneMap(colored);

	float3 gammaCorrectedColor = pow(ldrColor, 1.0f / 2.2f);
	return float4(gammaCorrectedColor, 0.0f);
}