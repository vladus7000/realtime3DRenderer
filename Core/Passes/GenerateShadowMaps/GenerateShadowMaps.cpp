#include "GenerateShadowMaps.hpp"
#include "Renderer.hpp"
#include "World.hpp"
#include "GBuffer.hpp"

GenerateShadowMaps::~GenerateShadowMaps()
{
}

void GenerateShadowMaps::setup(Renderer& renderer)
{
	const int depthSize = 1024;
	if (!m_depthTexture.m_DSV)
	{
		m_depthTexture.createDepthStencilTexture(depthSize, depthSize, renderer);
		renderer.registerTexture(Renderer::TextureResouces::ShadowMap, &m_depthTexture);
	}
}

void GenerateShadowMaps::release(Renderer& renderer)
{
	auto context = renderer.getContext();

}

void GenerateShadowMaps::draw(Renderer& renderer)
{
	auto context = renderer.getContext();
	auto& world = renderer.getWorld();

	auto& lights = renderer.getWorld().getLights();
	for (auto& l : lights)
	{
		if (l.m_type == Light::Type::Directional)
		{
			renderer.depthPrepass(l.m_camera, m_depthTexture, 0.0f, 0.0f, m_depthTexture.m_w, m_depthTexture.m_h);
			break; // 1 dir light supported
		}
	}
}

