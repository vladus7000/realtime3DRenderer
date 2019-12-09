#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Shader.hpp"
#include <string>
#include <vector>
#include "GBuffer.hpp"
#include "Texture.hpp"

class Pass;
class Window;
class World;

class Renderer
{
public:
	Renderer(Window& window, World& world);

	Renderer(const Renderer& rhs) = delete;
	Renderer(Renderer&& rhs) = delete;
	Renderer& operator=(const Renderer& rhs) = delete;
	Renderer& operator=(Renderer&& rhs) = delete;

	Shader createShader(const std::string& fileName, const std::string& vsName, const std::string& psName, std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {});

	void addMainPass(Pass* p);
	void addPostPass(Pass* p);

	void initialize();
	void deinitialize();

	void beginFrame();
	void drawFrame(float dt);
	void endFrame();

	ID3D11Device* getDevice() { return m_device; }
	ID3D11DeviceContext* getContext() { return m_context; }
	IDXGISwapChain* getSwapChain() { return m_swapChain; }

	ID3D11RenderTargetView* getDisplayBB() { return m_backBufferRT; }
	ID3D11Texture2D* getDisplayDepthStencil() { return m_gbuffer.m_depthStencilTexture; }
	ID3D11DepthStencilView* getDisplayDepthStencilView() { return m_gbuffer.m_depthStencilView; }

	D3D11_VIEWPORT& getMainViewport() { return m_viewport; }

	const World& getWorld() const { return m_world; }
	World& getWorld() { return m_world; }
	const Window& getWindow() const { return m_window; }

	GBuffer& getGBuffer() { return m_gbuffer; }
	Texture& getHDRTexture() { return m_hdrTexture; }

private:
	Window& m_window;
	World& m_world;

	IDXGIAdapter* m_adapter = nullptr; // null is primary
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext *m_context = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RenderTargetView* m_backBufferRT = nullptr;

	Texture m_hdrTexture;

	GBuffer m_gbuffer;

	D3D11_VIEWPORT m_viewport;

	std::vector<Pass*> m_mainPasses;
	std::vector<Pass*> m_postProcesses;
	int m_framesRendered = 0;
};