cbuffer globals
{
	float4 lightPosition_type;
	float3 lightDirection;
	float3 lightIntensity;
	float4x4 sunViewProjection;
	float4 blockPos;
}

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D positionMap : register(t2);
Texture2D shadowMap : register(t3);
TextureCube	environmentCube : register(t4);
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
	float3 position = positionMap.Sample(samplerState, tCoord).xyz;
	float intencity = 0.0f;

	float3 ambient = environmentCube.Sample(samplerState, normal).xyz;

	if (lightPosition_type.w == 0) // dir
	{
		float4 projectedPoint = mul(sunViewProjection, float4(position, 1.0f));
		projectedPoint.xyz /= projectedPoint.w;
		float bias = max(0.05 * (1.0 - dot(normalize(normal), normalize(-lightDirection))), 0.005);
		float z = projectedPoint.z;
		float2 newTCoord = projectedPoint.xy * 0.5f + float2(0.5, 0.5);
		newTCoord.y = 1.0f - newTCoord.y;
		//float zShadowMap = shadowMap.Sample(samplerState, newTCoord).x;

		float shadow = 0.0f;
		float2 texSize = float2(1.0 / 2048.0, 1.0 / 2048.0);
		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				float depth = shadowMap.Sample(samplerState, newTCoord + float2(x,y)*texSize ).x;
				shadow += (z - bias) < depth ? 1.0f : 0.0f;
			}
		}
		shadow /= 9.0f;
		if (shadow > 0.0f/*z <= zShadowMap*/)
		{
			intencity = shadow * saturate(dot(normalize(normal), normalize(-lightDirection)));
		}
	}
	else // point
	{
		float d = length(position - lightPosition_type.xyz);
		float maxR = dot(float3(0.2126f, 0.7152f, 0.0722f), lightIntensity);
		float att = 1.0f / (d * d);
		if (d < maxR);
		intencity = att/100000.0 * saturate(dot(normalize(normal), normalize(lightPosition_type.xyz - position)));
	}
	float3 color = (intencity * lightIntensity + ambient) * diffuse;
	return color;
}

float4 psmain(psInput input) : SV_Target
{
	float2 tcoord = input.tcoord*0.5f + 0.5f;
	tcoord.y = 1.0f - tcoord.y;
	
	float2 blockPosLocal = blockPos.xy;

	tcoord = lerp(float2(blockPosLocal), float2(blockPosLocal + blockPos.zw), tcoord);

	float3 hdrColor = litPixel(tcoord);
	return float4(hdrColor, 0.0f);
}