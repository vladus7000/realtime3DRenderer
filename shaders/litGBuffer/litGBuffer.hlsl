cbuffer globals
{
	float3 ambientLight;
	float3 lightPosition;
	float3 lightIntensity;
}

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D positionMap : register(t2);
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

float3 litPixel(float2 tCoord)
{
	float3 diffuse = diffuseMap.Sample(samplerState, tCoord).xyz;
	float3 normal = normalMap.Sample(samplerState, tCoord).xyz;
	//return normalize(normal);

	float intencity = saturate(dot(normalize(normal), normalize(lightPosition)));

	float3 color = intencity * diffuse * lightIntensity;
	float3 diffuseAmbient = diffuse * ambientLight;

	if (intencity > 0.5f)
	{
		return color;
	}
	else if(intencity > 0.01f)
	{
		return lerp(color, diffuseAmbient, intencity/4.0f);
	}
	else
	{
		return diffuseAmbient;
	}
}

float3 toneMap(float3 hdrColor)
{
	//https://www.shadertoy.com/view/lslGzl
	float exposure = 1.1f;
	hdrColor *= exposure / (1.0f + hdrColor / exposure);
	return hdrColor;
}

float4 psmain(psInput input) : SV_Target
{
	float2 tcoord = input.tcoord*0.5f + 0.5f;
	tcoord.y = 1.0f - tcoord.y;
	
	float3 hdrColor = litPixel(tcoord);

	float3 ldrColor = toneMap(hdrColor);

	float3 gammaCorrectedColor = pow(ldrColor, 1.0f / 2.2f);
	return float4(gammaCorrectedColor, 0.0f);
}