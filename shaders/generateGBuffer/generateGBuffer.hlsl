cbuffer globals
{
	float4x4 mvp;
	float4x4 mv;
}

Texture2D diffuseMap : register(t0);
SamplerState samplerState : register(s0);

struct vsInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 tcoords : TEXCOORDS;
};

struct psInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tcoord : TEXCOORD;
};
static matrix Identity =
{
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

psInput vsmain(vsInput input)
{
	psInput output;
	float4 p = float4(input.position, 1.0f);
	output.position = mul(mvp,p);
	output.normal = mul(mv, float4(input.position, 0.0f));
	output.tcoord = input.tcoords;
	return output;
}

struct ps_output
{
	float4 diffuse : SV_TARGET0;
	float4 position : SV_TARGET1;
	float4 normal : SV_TARGET2;
};

ps_output psmain(psInput input)
{
	ps_output output;

	output.diffuse =  diffuseMap.Sample(samplerState, input.tcoord);
	output.position = float4(input.position.xyz, 1.0f);
	output.normal = float4(input.normal, 1.0f);
	return output;
}