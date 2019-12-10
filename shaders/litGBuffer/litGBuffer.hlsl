cbuffer globals
{
	float3 ambientLight;
	float4 lightPosition_type;
	float3 lightDirection;
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
	//float3 diffuse = diffuseMap.Sample(samplerState, tCoord).xyz;
	float3 normal = normalMap.Sample(samplerState, tCoord).xyz;
	float3 position = positionMap.Sample(samplerState, tCoord).xyz;
	float intencity = 0.0f;

	if (lightPosition_type.w == 0) // dir
	{
		intencity = saturate(dot(normalize(normal), normalize(lightDirection)));
	}
	else // point
	{
		float d = length(position - lightPosition_type.xyz);
		float maxR = dot(float3(0.2126f, 0.7152f, 0.0722f), lightIntensity);
		float att = 1.0f / (d * d);
		if (d > maxR) discard;

		intencity = att * saturate(dot(normalize(normal), normalize(lightPosition_type.xyz - position)));
	}
	float3 color = intencity * /*diffuse*/ lightIntensity;
	return color;
}

float4 psmain(psInput input) : SV_Target
{
	float2 tcoord = input.tcoord*0.5f + 0.5f;
	tcoord.y = 1.0f - tcoord.y;
	
	float3 hdrColor = litPixel(tcoord);
	return float4(hdrColor, 0.0f);
}