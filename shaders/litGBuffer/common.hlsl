cbuffer lights
{
	float4 lightPosition_type[50];
	float3 lightDirection[50];
	float4 lightIntensity_radius[50];
	float4x4 sunViewProjection[3];
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4 viewPosition;
	float4 cascadeEndClip;
	float4 numLight_cascadeCount;
}

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D positionMap : register(t2);
Texture2D shadowMapC1[3] : register(t3);
//Texture2D shadowMapC2 : register(t4);
//Texture2D shadowMapC3 : register(t5);
TextureCube	environmentCube : register(t6);
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

float LinearizeDepth(float depth)
{
	float near_plane = 0.1f;
	float far_plane = 1000.0f;
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

int getShadowCascade(float3 position)
{
	float clipZ = mul(viewMatrix, float4(position, 1.0f)).z;
	float test[3];
	test[0] = cascadeEndClip.x;
	test[1] = cascadeEndClip.y;
	test[2] = cascadeEndClip.z;
	for (int i = 0; i < 3; i++)
	{
		if (clipZ <= cascadeEndClip[i])
		{
			return i;
		}
	}
	return 0;
}

float getShadowFromCascade(int cascade, float2 tcoord, float2 texelSize, float bias, float fragmentZ)
{
	float shadow = 0.0f;

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float depth = 0.0;
			switch (cascade)
			{
			case 0:
				depth = shadowMapC1[0].SampleLevel(samplerState, tcoord + float2(x, y)*texelSize, 0).x;
				break;
			case 1:
				depth = shadowMapC1[1].SampleLevel(samplerState, tcoord + float2(x, y)*texelSize, 0).x;
				break;
			case 2:
				depth = shadowMapC1[2].SampleLevel(samplerState, tcoord + float2(x, y)*texelSize, 0).x;
				break;
			}
			
			shadow += (fragmentZ - bias) < depth ? 1.0f : 0.0f;
		}
	}
	return shadow /= 9.0f;
}

float getShadowIntencity(float3 position, float bias)
{
	int cascade = getShadowCascade(position);
	float4 clipPoint = mul(sunViewProjection[cascade], float4(position, 1.0f));

	float3 projectedPoint = clipPoint.xyz / clipPoint.w;
	float z = projectedPoint.z;

	float2 newTCoord = projectedPoint.xy * 0.5f + float2(0.5, 0.5);
	newTCoord.y = 1.0f - newTCoord.y;

	if (z > 1.0 || z < 0) return 1.0;

	return getShadowFromCascade(cascade, newTCoord, float2(1.0 / 1024.0, 1.0 / 1024.0), bias, z);
}

float3 L(float3 position, float3 normal, int lightIndex)
{
	//float cascadeEnd[4] = { 0.1f, 50.0f, 200.0f, 1000.0f };
	float intencity = 0;
	if (lightPosition_type[lightIndex].w == 0) // dir
	{
		//float4 clipZ = mul(viewMatrix, float4(position, 1.0f));

		//float endClip1 = mul(projMatrix, float4(0.0, 0.0, cascadeEnd[1], 1.0f)).z;
		//float endClip2 = mul(projMatrix, float4(0.0, 0.0, cascadeEnd[2], 1.0f)).z;
		//float endClip3 = mul(projMatrix, float4(0.0, 0.0, cascadeEnd[3], 1.0f)).z;
		//bool CS1 = false;
		//bool CS2 = false;
		//bool CS3 = false;
		//
		//if (clipZ.z <= endClip1)
		//{
		//	CS1 = true;
		//}
		//else if (clipZ.z <= endClip2)
		//{
		//	CS2 = true;
		//}
		//else
		//{
		//	CS3 = true;
		//}

		//float4 projectedPoint1 = mul(sunViewProjection[0], float4(position, 1.0f));
		//float4 projectedPoint2 = mul(sunViewProjection[1], float4(position, 1.0f));
		//float4 projectedPoint3 = mul(sunViewProjection[2], float4(position, 1.0f));

		float bias = max(0.005 * (1.0 - dot(normalize(normal), normalize(-lightDirection[lightIndex]))), 0.0005);
		intencity = getShadowIntencity(position, bias);
		//projectedPoint1.xyz /= projectedPoint1.w;
		//float z1 = projectedPoint1.z;
		//float2 newTCoord1 = projectedPoint1.xy * 0.5f + float2(0.5, 0.5);
		//newTCoord1.y = 1.0f - newTCoord1.y;
		//
		//projectedPoint2.xyz /= projectedPoint2.w;
		//float z2 = projectedPoint2.z;
		//float2 newTCoord2 = projectedPoint2.xy * 0.5f + float2(0.5, 0.5);
		//newTCoord2.y = 1.0f - newTCoord2.y;
		//
		//
		//projectedPoint3.xyz /= projectedPoint3.w;
		//float z3 = projectedPoint3.z;
		//float2 newTCoord3 = projectedPoint3.xy * 0.5f + float2(0.5, 0.5);
		//newTCoord3.y = 1.0f - newTCoord3.y;



		//float zShadowMap = shadowMap.Sample(samplerState, newTCoord).x;

		//float shadow = 0.0f;
		//
		//for (int x = -1; x <= 1; x++)
		//{
		//	for (int y = -1; y <= 1; y++)
		//	{
		//		float depth;
		//		if (CS1)
		//		{
		//			float2 texSize = float2(1.0 / 1024.0, 1.0 / 1024.0);
		//			//colorD = float3(10.0, 0.0, 0.0);
		//			depth = shadowMapC1[0].SampleLevel(samplerState, newTCoord1 + float2(x, y)*texSize, 0).x;
		//			shadow += (z1 - bias) < depth ? 1.0f : 0.0f;
		//			if (z1 > 1) shadow = 0;
		//		}
		//		if (CS2)
		//		{
		//			float2 texSize = float2(1.0 / 1024.0, 1.0 / 1024.0);
		//			//colorD = float3(0.0, 10.0, 0.0);
		//			depth = shadowMapC1[1].SampleLevel(samplerState, newTCoord2 + float2(x, y)*texSize, 0).x;
		//			shadow += (z2 - bias) < depth ? 1.0f : 0.0f;
		//			if (z2 > 1) shadow = 0;
		//		}
		//		if (CS3)
		//		{
		//			float2 texSize = float2(1.0 / 512.0, 1.0 / 512.0);
		//			//colorD = float3(0.0, 0.0, 10.0);
		//			depth = shadowMapC1[2].SampleLevel(samplerState, newTCoord3 + float2(x, y)*texSize, 0).x;
		//			shadow += (z3 - bias) < depth ? 1.0f : 0.0f;
		//			if (z3 > 1) shadow = 0;
		//		}
		//		
		//	}
		//}
		//intencity =  shadow /= 9.0f;
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

	for (int lightIndex = 0; lightIndex < (int)numLight_cascadeCount.x; lightIndex++)
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