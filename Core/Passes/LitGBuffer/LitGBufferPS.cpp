#include "LitGBufferPS.hpp"
#include "Renderer.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Light.hpp"
#include "Resources.hpp"
#include "SettingsHolder.hpp"
#include "Settings/RenderSettings.hpp"

#include <algorithm>

LitGBufferPS::~LitGBufferPS()
{
}

void LitGBufferPS::setup(Renderer& renderer, Resources& resources)
{
	m_shadowMapC1 = resources.getResource<Texture>(Resources::ResoucesID::ShadowMapC1);
	m_shadowMapC2 = resources.getResource<Texture>(Resources::ResoucesID::ShadowMapC2);
	m_shadowMapC3 = resources.getResource<Texture>(Resources::ResoucesID::ShadowMapC3);

	m_cubeMap = resources.getResource<Texture>(Resources::ResoucesID::EnvironmentHDR);

	if (!m_lightsCB.buffer)
	{
		m_lightsCB = resources.createConstantBuffer<LightCB>();

		m_mainShader = resources.createShader("shaders/litGBuffer/litGBuffer.hlsl", "vsmain", "psmain");

		m_sampler = resources.getResource<ID3D11SamplerState>(Resources::ResoucesID::LinearSampler);
		m_depthState = resources.getResource<ID3D11DepthStencilState>(Resources::ResoucesID::NotEqualDepthState);
		m_blendState = resources.getResource<ID3D11BlendState>(Resources::ResoucesID::DefaultAdditiveBlend);
	}
}

void LitGBufferPS::release(Renderer& renderer, Resources& resources)
{
	auto context = renderer.getContext();

	ID3D11RenderTargetView* rtvs[] = { nullptr };
	context->OMSetRenderTargets(1, rtvs, nullptr);
	ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr, nullptr, nullptr, nullptr,nullptr,nullptr };
	context->PSSetShaderResources(0, 7, srvs);
	context->PSSetShader(nullptr, nullptr, 0);
	context->VSSetShader(nullptr, nullptr, 0);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	context->OMSetDepthStencilState(nullptr, 0);
	context->OMSetBlendState( nullptr, nullptr, 0xffffffff);
	ID3D11SamplerState* samplers[] = { nullptr };
	context->PSSetSamplers(0, 1, samplers);

	auto& viewport = renderer.getMainViewport();
	context->RSSetViewports(1, &viewport);
}

void LitGBufferPS::execute(Renderer& renderer)
{
	auto context = renderer.getContext();
	auto& world = renderer.getWorld();

	auto& gbuffer = renderer.getGBuffer();
	auto settings = SettingsHolder::getInstance().getSetting<RenderSettings>(Settings::Type::Render);

	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(renderer.getHDRTexture().m_RT.Get(), color);

	ID3D11RenderTargetView* rtvs[] = { renderer.getHDRTexture().m_RT.Get() };

	context->OMSetRenderTargets(1, rtvs, gbuffer.m_depth.m_DSV.Get());
	ID3D11ShaderResourceView* srvs[] = { gbuffer.m_diffuse.m_SRV.Get(), gbuffer.m_normal_metalnes.m_SRV.Get(),
		gbuffer.m_position_rough.m_SRV.Get(),
		m_shadowMapC1->m_SRV.Get(), m_shadowMapC2->m_SRV.Get(),m_shadowMapC3->m_SRV.Get(), m_cubeMap->m_SRV.Get() };
	context->PSSetShaderResources(0, 7 , srvs);

	//TODO set sampler for cube map
	context->PSSetSamplers(0, 1, &m_sampler);

	context->OMSetDepthStencilState(m_depthState, 0);

	context->RSSetViewports(1, &renderer.getMainViewport());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(nullptr);
	context->PSSetShader(m_mainShader.getPixelShader(), nullptr, 0);
	context->VSSetShader(m_mainShader.getVertexShader(), nullptr, 0);

	context->OMSetBlendState(m_blendState, nullptr, 0xffffffff);

	std::vector<glm::vec4> screenBlocks;
	float stepX = 800.0 / 10.f;
	float stepY = 600.0 / 10.f;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			screenBlocks.push_back(glm::vec4(i*stepX, j*stepY, i*stepX + stepX, j*stepY + stepY));
		}
	}
	D3D11_VIEWPORT m_viewport;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	//context->RSSetViewports(1, &m_viewport);
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;

	//for (auto& sBlock : screenBlocks)
	{
		m_viewport.Width = 80.0f;
		m_viewport.Height = 60.0f;
		//m_viewport.TopLeftX = sBlock.x;
		//m_viewport.TopLeftY = sBlock.y;
	//	context->RSSetViewports(1, &m_viewport);

		auto buffer = renderer.lockConstantBuffer(m_lightsCB);
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

			memcpy(&buffer->pos_type[lightIndex*4], &l.m_position[0], sizeof(float[3]));
			buffer->pos_type[lightIndex * 4 + 3] = l.m_type == Light::Type::Directional ? 0.0f : 1.0f;

			memcpy(&buffer->dir[lightIndex * 4], &l.m_direction[0], sizeof(float[3]));
			
			memcpy(&buffer->intensity[lightIndex * 4], &l.m_intensity[0], sizeof(float[3]));
			buffer->intensity[lightIndex * 4 + 3] = l.m_radius;

			if (l.m_type == Light::Type::Directional)
			{
				glm::mat4 mvp = settings->cascadesProjectionMatrix[0] * l.m_camera.getView();
				memcpy(&buffer->sunViewProjection[16 * 0], &mvp[0][0], sizeof(float[16]));

				mvp = settings->cascadesProjectionMatrix[1] * l.m_camera.getView();
				memcpy(&buffer->sunViewProjection[16 * 1], &mvp[0][0], sizeof(float[16]));

				mvp = settings->cascadesProjectionMatrix[2] * l.m_camera.getView();
				memcpy(&buffer->sunViewProjection[16 * 2], &mvp[0][0], sizeof(float[16]));

				memcpy(buffer->cascadeEndClip, settings->cascadesEndClip, sizeof(float) * settings->cascadesCount);
			}

			lightIndex++;
		}

		memcpy(buffer->viewPosition, &world.getCamera().getPosition()[0], sizeof(float[3]));
		auto wp = world.getCamera().getProjection() * world.getCamera().getView();
		memcpy(buffer->viewMatrix, &wp[0][0], sizeof(float[16]));
		memcpy(buffer->projMatrix, &world.getCamera().getProjection()[0][0], sizeof(float[16]));

		lightIndex = std::max(lightIndex, m_lightMaxNumber);
		buffer->numLights_csCount[0] = lightIndex;
		buffer->numLights_csCount[1] = settings->cascadesCount;
		renderer.unlockConstantBuffer(m_lightsCB);

		ID3D11Buffer* constants[] = { m_lightsCB.buffer.Get() };
		context->PSSetConstantBuffers(0, 1, constants);
		context->VSSetConstantBuffers(0, 1, constants);
		context->Draw(4, 0);
	}
}

