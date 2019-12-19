#include "Renderer.hpp"
#include "Pass.hpp"
#include "Window.hpp"
#include "World.hpp"
#include "Camera.hpp"
#include "Resources.hpp"

#include "Passes/GenerateGBuffer/GenerateGbuffer.hpp"
#include "Passes/LitGBuffer/LitGBufferCS.hpp"
#include "Passes/LitGBuffer/LitGBufferPS.hpp"
#include "Passes/GenerateShadowMaps/GenerateShadowMaps.hpp"
#include "Passes/SkyBox/SkyAndFog.hpp"
#include "Passes/Tonemap/LuminanceAvg.hpp"
#include "Passes/Bloom/Bloom.hpp"
#include "Passes/FinalPost/FinalPost.hpp"
#include "Passes/MainZpass/MainZpass.hpp"
#include "Passes/SpotlightSM/SpotlightSM.hpp"
#include "Passes/SSAO/SSAO.hpp"
#include "Passes/SSR/SSR.hpp"

#include "SettingsHolder.hpp"
#include "Settings/RenderSettings.hpp"

Renderer::Renderer(Window& window, Resources& resources)
	: m_window(window)
	, m_resources(resources)
	, m_context(resources.getContext())
	, m_device(resources.getDevice())
	, m_gbuffer(*this, resources)
{
	initialize();
}

Renderer::~Renderer()
{
	deinitialize();
}

void Renderer::initialize()
{
	//swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = m_window.getWidth();
	swapChainDesc.BufferDesc.Height = m_window.getHeight();
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = m_window.getHWND();
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	//unsigned int quality;
	//device->CheckMultisampleQualityLevels(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, 4, &quality);

	IDXGIDevice* dxgiDevice = nullptr;
	m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	IDXGIAdapter* dxgiAdapter = nullptr;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	IDXGIFactory* dxgiFactory = nullptr;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);


	dxgiFactory->CreateSwapChain(m_device, &swapChainDesc, &m_swapChain);

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	//Get BB
	ID3D11Texture2D* buffer = nullptr;
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);
	m_device->CreateRenderTargetView(buffer, nullptr, &m_backBufferRT);
	buffer->Release();

	m_viewport.Width = (float)m_window.getWidth();
	m_viewport.Height = (float)m_window.getHeight();
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	///-----
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Width = m_window.getWidth();
		desc.Height = m_window.getHeight();
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		m_device->CreateTexture2D(&desc, nullptr, &m_hdrTexture.m_texture);
		m_device->CreateShaderResourceView(m_hdrTexture.m_texture.Get(), nullptr, m_hdrTexture.m_SRV.GetAddressOf());
		m_device->CreateRenderTargetView(m_hdrTexture.m_texture.Get(), nullptr, m_hdrTexture.m_RT.GetAddressOf());
		m_device->CreateUnorderedAccessView(m_hdrTexture.m_texture.Get(), nullptr, m_hdrTexture.m_UAV.GetAddressOf());
	}

    { // depth prepass
        m_depthPrepassShader = m_resources.createShader("shaders/generateGBuffer/generateGBuffer.hlsl", "vsmain", "");

        D3D11_BUFFER_DESC buffDesc;
        buffDesc.ByteWidth = sizeof(float[16]) * 2;
        buffDesc.Usage = D3D11_USAGE_DYNAMIC;
        buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        buffDesc.MiscFlags = 0;
        m_device->CreateBuffer(&buffDesc, nullptr, &m_depthPrepassCB);
    }
	///////////////////////

	{// env texture
		m_envHDR = m_resources.loadTexture("stpeters_cross.dds");
		m_resources.registerResource(Resources::ResoucesID::EnvironmentHDR, &m_envHDR);
	}

	{
		m_cubeMapDay = m_resources.loadTexture("stpeters_cross.dds");//desert
		m_resources.registerResource(Resources::ResoucesID::EnvCubeMapDay, &m_cubeMapDay);
		m_cubeMapNight = m_resources.loadTexture("stpeters_cross.dds");//moondust
		m_resources.registerResource(Resources::ResoucesID::EnvCubeMapNight, &m_cubeMapNight);
	}

	m_mainGBuffer.reset(new GenerateGBuffer{});
	m_cascadedShadowMaps.reset(new GenerateShadowMaps{});
	m_lightPassBeginCS.reset(new LitGBufferCS{});
	m_lightPassBeginPS.reset(new LitGBufferPS{});
	m_luminanceAvg.reset(new LuminanceAvg{});
	m_skyAndFog.reset(new SkyAndFog{});
	m_mainZpass.reset(new MainZpass{});
	m_spotlightShadowMaps.reset(new SpotlightSM{});
	m_bloom.reset(new Bloom{});
	m_finalPost.reset(new FinalPost{});
	m_ssao.reset(new SSAO{});
	m_ssr.reset(new SSR{});

	m_framePasses.reserve(12);

	SettingsHolder::getInstance().addSetting(Settings::Type::Render, new RenderSettings{});
}

void Renderer::deinitialize()
{
	m_framePasses.clear();
    m_depthPrepassCB->Release();
	m_backBufferRT->Release();
	m_swapChain->Release();
}

void Renderer::constructPasses()
{
	m_framePasses.push_back(m_mainZpass.get());
	m_framePasses.push_back(m_mainGBuffer.get());

	m_framePasses.push_back(m_cascadedShadowMaps.get());
	m_framePasses.push_back(m_spotlightShadowMaps.get());

	m_framePasses.push_back(m_ssao.get());

	auto settings = SettingsHolder::getInstance().getSetting<RenderSettings>(Settings::Type::Render);
	if (settings->useCSforLighting)
	{
		m_framePasses.push_back(m_lightPassBeginCS.get());
	}
	else
	{
		m_framePasses.push_back(m_lightPassBeginPS.get());
	}

	m_framePasses.push_back(m_ssr.get());

	m_framePasses.push_back(m_bloom.get());
	m_framePasses.push_back(m_luminanceAvg.get());
	m_framePasses.push_back(m_skyAndFog.get());
	m_framePasses.push_back(m_finalPost.get());
}

void Renderer::beginFrame()
{
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };// 0.3 0.3 0.6
	m_context->ClearRenderTargetView(m_backBufferRT, color);
	m_context->ClearDepthStencilView(m_gbuffer.m_depth.m_DSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_context->OMSetRenderTargets(1, &m_backBufferRT, m_gbuffer.m_depth.m_DSV.Get());
	m_context->RSSetViewports(1, &m_viewport);

	constructPasses();
}

void Renderer::drawFrame(float dt)
{
	for (auto pass : m_framePasses)
	{
		pass->setup(*this, m_resources);
		pass->execute(*this);
		pass->release(*this, m_resources);
	}

	m_swapChain->Present(1, 0);

}

void Renderer::endFrame()
{
	m_framePasses.clear();
	m_framesRendered++;
}

void Renderer::depthPrepass(const Camera& camera, Texture& tex, float x, float y, float w, float h)
{
    m_context->ClearDepthStencilView(tex.m_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    ID3D11RenderTargetView* rtvs[] = { nullptr };
    m_context->OMSetRenderTargets(1, rtvs, tex.m_DSV.Get());

    m_context->VSSetShader(m_depthPrepassShader.getVertexShader(), nullptr, 0);
    m_context->PSSetShader(nullptr, nullptr, 0);
    m_context->IASetInputLayout(m_depthPrepassShader.getInputLayout());

    m_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer* constants[] = { m_depthPrepassCB };
    m_context->VSSetConstantBuffers(0, 1, constants);

	m_viewport.Width = w;
	m_viewport.Height = h;
	m_viewport.TopLeftX = x;
	m_viewport.TopLeftY = y;
	m_context->RSSetViewports(1, &m_viewport);
    for (auto& mesh : m_world->getObjects())
    {
        D3D11_MAPPED_SUBRESOURCE res;
        m_context->Map(m_depthPrepassCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
        struct Data
        {
            float mvp[16];
            float model[16];
        };
        Data* buffer = reinterpret_cast<Data*>(res.pData);

        glm::mat4 mvp = camera.getProjection() * camera.getView() * mesh.worldMatrix;

        memcpy(buffer->mvp, &mvp[0][0], sizeof(float[16]));
        memcpy(buffer->model, &(mesh.worldMatrix[0][0]), sizeof(float[16]));
        m_context->Unmap(m_depthPrepassCB, 0);

        unsigned int stride[] = { sizeof(float[3]), sizeof(float[3]) , sizeof(float[2]) };
        unsigned int offsets[] = { 0, 0, 0 };
        ID3D11Buffer* buffers[] = { mesh.vert_vb, mesh.norm_vb, mesh.tcoords_vb };
        m_context->IASetVertexBuffers(0, 3, buffers, stride, offsets);
        m_context->Draw(mesh.numIndices, 0);
    }

	m_viewport.Width = (float)m_window.getWidth();
	m_viewport.Height = (float)m_window.getHeight();
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_context->RSSetViewports(1, &m_viewport);

	ID3D11Buffer* buffers[] = { nullptr };
	m_context->IASetVertexBuffers(0, 0, buffers, nullptr, nullptr);
	m_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);

	{
		ID3D11Buffer* constants[] = { nullptr };
		m_context->VSSetConstantBuffers(0, 1, constants);
	}
	m_context->VSSetShader(nullptr, nullptr, 0);
	m_context->PSSetShader(nullptr, nullptr, 0);
	m_context->IASetInputLayout(nullptr);
	
	m_context->OMSetRenderTargets(1, rtvs, nullptr);
}
