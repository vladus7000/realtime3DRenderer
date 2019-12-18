#pragma once

#include "Shader.hpp"
#include <string>
#include <vector>
#include "GBuffer.hpp"
#include "Texture.hpp"
#include <map>
#include <memory>

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

	void beginFrame();
	void drawFrame(float dt);
	void endFrame();

    void depthPrepass(const Camera& camera, Texture& tex, float x, float y, float w, float h);

	Resources& getResources() { return m_resources; }
	ID3D11DeviceContext* getContext() { return m_context; }

	ID3D11RenderTargetView* getDisplayBB() { return m_backBufferRT; }
	ID3D11Texture2D* getDisplayDepthStencil() { return m_gbuffer.m_depth.m_texture.Get(); }
	ID3D11DepthStencilView* getDisplayDepthStencilView() { return m_gbuffer.m_depth.m_DSV.Get(); }

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
	void constructPasses();

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
	Texture m_cubeMapDay;
	Texture m_cubeMapNight;
	D3D11_VIEWPORT m_viewport;
	//TODO:
	//reflectionCapture, planarReflections, dynamicEnvMap, (mainZPass), (mainGBuffer), mainGBufferSimple, mainGBufferDecals, decalVolumes, mainGBufferFixup,
	//msaaZDown, msaaClassify, lensFlaresOcclusionQueries, (lightPassBegin), (cascadedShadowMaps), (spotlightShadowMaps), downsampleZ, linearizeZ, (ssao),ssaoFilter, (ssr),
	//halfRezZPass, halfResTransparent, mainDistort, lightPassEnd, mainOpaque, linearizeZ, mainOpaqueEmmisive, mainTransDecal, fgOpaqueEmissive, subsurfaceScattering,
	//(skyAndFog), hairCoverage, mainTransDepth,linearizeZ, mainTransparent, halfResUpsample, motionBloorDerive, motionBloorVelocity, motionBoolFilter, filmicEffectsEdge,
	//spriteDof, fgTransparent, lensScope, filmicEffects, (bloom), (luminanceAvg), (finalPost), overlay, fxaa, smaa, resample, screenEffect
	std::unique_ptr<Pass> m_mainZpass;
	std::unique_ptr<Pass> m_mainGBuffer;
	std::unique_ptr<Pass> m_cascadedShadowMaps;
	std::unique_ptr<Pass> m_spotlightShadowMaps;
	std::unique_ptr<Pass> m_lightPassBeginCS;
	std::unique_ptr<Pass> m_lightPassBeginPS;
	std::unique_ptr<Pass> m_bloom;
	std::unique_ptr<Pass> m_luminanceAvg;
	std::unique_ptr<Pass> m_skyAndFog;
	std::unique_ptr<Pass> m_finalPost;
	std::unique_ptr<Pass> m_ssao;
	std::unique_ptr<Pass> m_ssr;

	std::vector<Pass*> m_framePasses;
	int m_framesRendered = 0;
};