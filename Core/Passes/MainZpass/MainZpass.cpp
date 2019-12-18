#include "MainZpass.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Resources.hpp"

MainZpass::~MainZpass()
{
}

void MainZpass::setup(Renderer& renderer, Resources& resources)
{
}

void MainZpass::release(Renderer& renderer, Resources& resources)
{
}

void MainZpass::execute(Renderer& renderer)
{
	auto context = renderer.getContext();
	auto& world = renderer.getWorld();
	auto& gbuffer = renderer.getGBuffer();
	gbuffer.clearDepth(renderer);

	auto& camera = renderer.getWorld().getCamera();
	renderer.depthPrepass(camera, gbuffer.m_depth, 0.0f, 0.0f, gbuffer.m_depth.m_w, gbuffer.m_depth.m_h);
}
