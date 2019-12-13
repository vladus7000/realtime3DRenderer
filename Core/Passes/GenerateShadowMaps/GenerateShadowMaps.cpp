#include "GenerateShadowMaps.hpp"
#include "Renderer.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Resources.hpp"

GenerateShadowMaps::~GenerateShadowMaps()
{
}

void GenerateShadowMaps::setup(Renderer& renderer, Resources& resources)
{
	const int depthSize = 2048;
	if (!m_depthTexture.m_DSV)
	{
		m_depthTexture = resources.createDepthStencilTexture(depthSize, depthSize);
		resources.registerTexture(Resources::TextureResouces::ShadowMap, &m_depthTexture);
	}
}

void GenerateShadowMaps::release(Renderer& renderer, Resources& resources)
{
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

