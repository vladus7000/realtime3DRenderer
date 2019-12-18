cbuffer lights
{
	float4 lightPosition_type[50];
	float3 lightDirection[50];
	float4 lightIntensity_radius[50];
	float4x4 sunViewProjection;
	float4 viewPosition;
	float4 numLight;
}

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D positionMap : register(t2);
Texture2D shadowMap : register(t3);
TextureCube	environmentCube : register(t4);
SamplerState samplerState : register(s0);


float DistGGX(float3 N, float3 H, float a)
{
	float a2 = a * a * a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = 3.141592 * denom * denom;
	return nom / denom;
}

float GeomSchlickGGX(float NdotV, float roughness)
{
	//float nom = NdotV;
	//float denom = NdotV * (1.0 - k) + k;
	//return nom / denom;

	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeomSmith(float3 N, float3 V, float3 L, float k)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);

	float ggx1 = GeomSchlickGGX(NdotV, k);
	float ggx2 = GeomSchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 freshelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 BRDF(float3 position, float3 normal, float3 Wi, float3 Wo, float3 albedo, float m, float r)
{
	float3 Ks, Kd;
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, m);
	float3 H = normalize(Wi + Wo);
	float cosT = saturate(dot(H, Wi));

	float3 F = freshelSchlick(cosT, F0);
	Ks = F;
	Kd = float3(1.0, 1.0, 1.0) - Ks;
	Kd *= 1.0 - m;
	float3 lambert = Kd * albedo / 3.1415;

	float3 cookTorrance = Ks * DistGGX(normal, H, r) * GeomSmith(normal, Wo, Wi, r) * F;
	float denom = 4.0 * saturate(dot(Wo, normal)) * saturate(dot(Wi, normal));

	cookTorrance = cookTorrance / max(denom, 0.001);
	return lambert + cookTorrance;
}

float3 L(float3 position, float3 normal, int lightIndex)
{
	float intencity = 0;
	if (lightPosition_type[lightIndex].w == 0) // dir
	{
		float4 projectedPoint = mul(sunViewProjection, float4(position, 1.0f));
		projectedPoint.xyz /= projectedPoint.w;
		float bias = max(0.05 * (1.0 - dot(normalize(normal), normalize(-lightDirection[lightIndex]))), 0.005);
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
				float depth = shadowMap.SampleLevel(samplerState, newTCoord + float2(x, y)*texSize, 0).x;
				shadow += (z - bias) < depth ? 1.0f : 0.0f;
			}
		}
		intencity = shadow /= 9.0f;
	}
	else // point
	{
		float r = lightIntensity_radius[lightIndex].w;
		float d = length(lightPosition_type[lightIndex].xyz - position);
		//float maxR = dot(float3(0.2126f, 0.7152f, 0.0722f), lightIntensity_radius[lightIndex].xyz);
		float att = 1.0f / (1.0f + d * d);
		//if (d < maxR);
		intencity = att;
	}

	return intencity * lightIntensity_radius[lightIndex].xyz;
}

float3 litPixel(float2 tCoord)
{
	float3 diffuse = diffuseMap.SampleLevel(samplerState, tCoord, 0).xyz;
	float4 normal_m = normalMap.SampleLevel(samplerState, tCoord, 0);
	float4 position_r = positionMap.SampleLevel(samplerState, tCoord, 0);
	float m = normal_m.w;
	float r = position_r.w;

	float3 color = float3(0, 0, 0);
	float3 ambient = environmentCube.SampleLevel(samplerState, normal_m.xyz, 0).xyz;

	for (int lightIndex = 0; lightIndex < (int)numLight.x; lightIndex++)
	{
		float NdotL = saturate(dot(normal_m.xyz, normalize(lightPosition_type[lightIndex].xyz - position_r.xyz)));
		color += BRDF(position_r.xyz,
			normalize(normal_m.xyz),
			normalize(lightPosition_type[lightIndex].xyz - position_r.xyz), //Wi
			normalize(viewPosition - position_r.xyz), // Wo
			diffuse, m, r
		) * L(position_r.xyz, normal_m.xyz, lightIndex) * NdotL;

		//color += intencity * lightIntensity_radius[lightIndex].xyz * diffuse;
	}
	color += BRDF(position_r.xyz,
		normalize(normal_m.xyz),
		normalize((position_r.xyz + normal_m.xyz) * 5.0 - position_r.xyz),
		normalize(viewPosition - position_r.xyz),
		diffuse, m, r
	) * ambient;

	return color;
}