#pragma once

#include "Shader.hpp"
#include <string>
#include <vector>
#include "GBuffer.hpp"
#include "Texture.hpp"
#include <map>

class Pass;
class Window;
class World;
class Camera;
class Resources;

class Renderer
{
public:
	Renderer(Window& window, Resources& resources);
	~Renderer();

	Renderer(const Renderer& rhs) = delete;
	Renderer(Renderer&& rhs) = delete;
	Renderer& operator=(const Renderer& rhs) = delete;
	Renderer& operator=(Renderer&& rhs) = delete;

	void addMainPass(Pass* p);
	void addPostPass(Pass* p);

	void beginFrame();
	void drawFrame(float dt);
	void endFrame();

    void depthPrepass(const Camera& camera, Texture& tex, float x, float y, float w, float h);

	Resources& getResources() { return m_resources; }
	ID3D11DeviceContext* getContext() { return m_context; }

	ID3D11RenderTargetView* getDisplayBB() { return m_backBufferRT; }
	ID3D11Texture2D* getDisplayDepthStencil() { return m_gbuffer.m_depthStencilTexture; }
	ID3D11DepthStencilView* getDisplayDepthStencilView() { return m_gbuffer.m_depthStencilView; }

	D3D11_VIEWPORT& getMainViewport() { return m_viewport; }

	void setWorld(World* w) { m_world = w; }
	const World& getWorld() const { return *m_world; }
	World& getWorld() { return *m_world; }
	const Window& getWindow() const { return m_window; }

	GBuffer& getGBuffer() { return m_gbuffer; }
	Texture& getHDRTexture() { return m_hdrTexture; }

private:
	void initialize();
	void deinitialize();

private:
	Window& m_window;
	World* m_world;
	Resources& m_resources;

	IDXGIAdapter* m_adapter = nullptr; // null is primary
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext *m_context = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RenderTargetView* m_backBufferRT = nullptr;

	Texture m_hdrTexture;
    Shader m_depthPrepassShader;
    ID3D11Buffer* m_depthPrepassCB = nullptr;
	GBuffer m_gbuffer;
	Texture m_envHDR;
	Texture m_cubeMap;
	D3D11_VIEWPORT m_viewport;

	std::vector<Pass*> m_mainPasses;
	std::vector<Pass*> m_postProcesses;
	int m_framesRendered = 0;
};