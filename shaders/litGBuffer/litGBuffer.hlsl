cbuffer globals
{
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
	//float4 p = float4(input.position, 0.0f, 1.0f);
	
	float2 texcoord = float2(input.index & 1, input.index >> 1);
	output.position = float4((texcoord.x - 0.5f) * 2, -(texcoord.y - 0.5f) * 2, 1, 1);
	output.tcoord = output.position.xy;
	return output;
}

float4 psmain(psInput input) : SV_Target
{
	float2 tcoord = input.tcoord*0.5f + 0.5f;
	tcoord.y = 1.0f - tcoord.y;
	
	float3 diffuse = diffuseMap.Sample(samplerState, tcoord).xyz;
	float3 normal = normalMap.Sample(samplerState, tcoord).xyz;
	//float3 lightPos(50.0f, 25.0f, 0.0f);
	float intencity = saturate(dot(normalize(normal), normalize(lightPosition) ) );
	return float4(intencity * diffuse * lightIntensity/*normalize(input.normal)*/ + float3(0.2f, 0.2f, 0.2f), 0.5f);
}