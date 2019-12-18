#include "LitGBufferCS.hpp"
#include "Renderer.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Light.hpp"
#include "Resources.hpp"

#include <algorithm>

LitGBufferCS::~LitGBufferCS()
{
	if (m_lightsCB)
	{
		m_lightsCB->Release();
	}

	m_sampler->Release();
}

void LitGBufferCS::setup(Renderer& renderer, Resources& resources)
{
	m_shadowMap = resources.getTextureResource(Resources::TextureResouces::ShadowMap);
	m_cubeMap = resources.getTextureResource(Resources::TextureResouces::EnvironmentHDR);

	if (!m_lightsCB)
	{
		auto device = resources.getDevice();
		D3D11_BUFFER_DESC buffDesc;
		buffDesc.ByteWidth = sizeof(float[4]) * 3 * m_lightMaxNumber + sizeof(float[4]) * 2 + sizeof(float[16]); // must be multiply of 16
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		device->CreateBuffer(&buffDesc, nullptr, &m_lightsCB);

		D3D11_SAMPLER_DESC sampler;
		ZeroMemory(&sampler, sizeof(D3D11_SAMPLER_DESC));
		sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		device->CreateSamplerState(&sampler, &m_sampler);

		m_csShader = resources.createComputeShader("shaders/litGBuffer/litGbuffer_c.hlsl", "csMain");
	}
}

void LitGBufferCS::release(Renderer& renderer, Resources& resources)
{
	auto context = renderer.getContext();

	context->CSSetShader(nullptr, nullptr, 0);
	ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	context->CSSetShaderResources(0, 5, srvs);
	ID3D11UnorderedAccessView* uavs[] = { nullptr };
	context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

}

void LitGBufferCS::execute(Renderer& renderer)
{
	auto context = renderer.getContext();
	auto& world = renderer.getWorld();

	auto& gbuffer = renderer.getGBuffer();
	//TODO set sampler for cube map
	context->CSSetSamplers(0, 1, &m_sampler);

	context->CSSetShader(m_csShader.getComputeShader(), nullptr, 0);

	ID3D11ShaderResourceView* srvs[] = { gbuffer.m_diffuse.m_SRV.Get(), gbuffer.m_normal_metalnes.m_SRV.Get(), gbuffer.m_position_rough.m_SRV.Get(), m_shadowMap->m_SRV.Get(), m_cubeMap->m_SRV.Get() };
	context->CSSetShaderResources(0, 5, srvs);
	ID3D11UnorderedAccessView* uavs[] = { renderer.getHDRTexture().m_UAV.Get() };
	context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

	D3D11_MAPPED_SUBRESOURCE res;
	context->Map(m_lightsCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	struct Data
	{
		float pos_type[4 * m_lightMaxNumber];
		float dir[4 * m_lightMaxNumber];
		float intensity[4 * m_lightMaxNumber];
		float sunViewProjection[16];
		float viewPosition[4];
		float numLights[4];
	};
	Data* buffer = reinterpret_cast<Data*>(res.pData);
	int lightIndex = 0;
	auto& lights = renderer.getWorld().getLights();
	for (int i = 0; i < std::min((int)lights.size(), m_lightMaxNumber); i++)
	{
		auto& l = lights[i];
		if (!l.enabled)
		{
			continue;
		}
		float radius = glm::dot(glm::vec3(0.2126f, 0.7152f, 0.0722f), l.m_intensity);
		float lightX = l.m_position.x;
		float lightY = l.m_position.y;

		memcpy(&buffer->pos_type[lightIndex * 4], &l.m_position[0], sizeof(float[3]));
		buffer->pos_type[lightIndex * 4 + 3] = l.m_type == Light::Type::Directional ? 0.0f : 1.0f;

		memcpy(&buffer->dir[lightIndex * 4], &l.m_direction[0], sizeof(float[3]));

		memcpy(&buffer->intensity[lightIndex * 4], &l.m_intensity[0], sizeof(float[3]));
		buffer->intensity[lightIndex * 4 + 3] = l.m_radius;

		if (l.m_type == Light::Type::Directional)
		{
			glm::mat4 mvp = l.m_camera.getProjection() * l.m_camera.getView();
			memcpy(buffer->sunViewProjection, &mvp[0][0], sizeof(float[16]));
		}
		lightIndex++;
	}

	memcpy(buffer->viewPosition, &world.getCamera().getPosition()[0], sizeof(float[3]));

	lightIndex = std::max(lightIndex, m_lightMaxNumber);
	buffer->numLights[0] = lightIndex;
	context->Unmap(m_lightsCB, 0);
	ID3D11Buffer* constants[] = { m_lightsCB };
	context->CSSetConstantBuffers(0, 1, constants);
	context->Dispatch(800 / 20, 600 / 20, 1);
}

