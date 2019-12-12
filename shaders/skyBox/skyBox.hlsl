cbuffer ShaderData : register(b0)
{
	float4x4 cameraViewMatrix;
	float4 sunAngle;
};

TextureCube	environment : register(t0);
TextureCube	environmentNight : register(t1);
SamplerState sampl : register(s0);

struct VertexShaderInput
{
	float4 pos : POSITION;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 texCoord : TEXCOORD0;
};

PixelShaderInput vsmain(VertexShaderInput input)
{
	PixelShaderInput output;
	output.pos = input.pos;
	output.texCoord = normalize(mul(cameraViewMatrix, input.pos.xyz));

	return output;
}

float3 toneMap(float3 hdrColor)
{
	//https://www.shadertoy.com/view/lslGzl
	float exposure = 1.1f;
	hdrColor *= exposure / (1.0f + hdrColor / exposure);
	return hdrColor;
}

float4 psmain(PixelShaderInput input) : SV_TARGET
{
	float4 color1 = environment.Sample(sampl, input.texCoord);
	float4 color2 = environmentNight.Sample(sampl, input.texCoord);
	float4 color;
	if (sunAngle.x >= 90.0 && sunAngle.x < 180.0)
		color = lerp(color1, color2, (sunAngle.x - 90.0) / 90.0);
	else if (sunAngle.x >= 270.0 && sunAngle.x < 360)
		color = lerp(color2, color1, (sunAngle.x - 270.0) / 90.0);
	else
	{
		if (sunAngle.x < 90.0)
		{
			color = color1;
		}
		else if (sunAngle.x < 270.0)
		{
			color = color2;
		}
	}
	//float3 ldrColor = toneMap(color);

	//float3 gammaCorrectedColor = pow(ldrColor, 1.0f / 2.2f);
	return color;
}