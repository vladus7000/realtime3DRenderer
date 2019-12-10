#include "Renderer.hpp"
#include <fstream>
#include <streambuf>
#include <iostream>
#include "Pass.hpp"
#include "Window.hpp"
#include "World.hpp"
#include "Camera.hpp"

#include "Passes/GenerateGBuffer/GenerateGbuffer.hpp"
#include "Passes/LitGBuffer/LitGBuffer.hpp"

Renderer::Renderer(Window& window,  World& world)
	: m_window(window)
	, m_world(world)
{

}

void Renderer::initialize()
{
	D3D_FEATURE_LEVEL level[] = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL outLevel;
	unsigned int flags = D3D11_CREATE_DEVICE_DEBUG;
	D3D11CreateDevice(m_adapter, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, level, 1, D3D11_SDK_VERSION, &m_device, &outLevel, &m_context);

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

	m_gbuffer.initialize(*this);

	m_viewport.Width = m_window.getWidth();
	m_viewport.Height = m_window.getHeight();
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	///-----
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
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
		m_device->CreateShaderResourceView(m_hdrTexture.m_texture, nullptr, &m_hdrTexture.m_SRV);
		m_device->CreateRenderTargetView(m_hdrTexture.m_texture, nullptr, &m_hdrTexture.m_RT);
	}

    { // depth prepass
        m_depthPrepassShader = createShader("shaders/generateGBuffer/generateGBuffer.hlsl", "vsmain", "");

        D3D11_BUFFER_DESC buffDesc;
        buffDesc.ByteWidth = sizeof(float[16]) * 2;
        buffDesc.Usage = D3D11_USAGE_DYNAMIC;
        buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        buffDesc.MiscFlags = 0;
        m_device->CreateBuffer(&buffDesc, nullptr, &m_depthPrepassCB);
    }
	///////////////////////
	addMainPass(new GenerateGBuffer{});
	addMainPass(new LitGBuffer{});
	// Tonemap pass
}

void Renderer::deinitialize()
{
	for (auto pass : m_mainPasses)
	{
		delete pass;
	}

	for (auto post : m_postProcesses)
	{
		delete post;
	}

    m_depthPrepassCB->Release();
	m_mainPasses.clear();
	m_postProcesses.clear();
	m_gbuffer.release();
	m_backBufferRT->Release();
	m_swapChain->Release();
	m_device->Release();
	m_context->Release();
}

void Renderer::beginFrame()
{
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };// 0.3 0.3 0.6
	m_context->ClearRenderTargetView(m_backBufferRT, color);
	m_context->ClearDepthStencilView(m_gbuffer.m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_context->OMSetRenderTargets(1, &m_backBufferRT, m_gbuffer.m_depthStencilView);
	m_context->RSSetViewports(1, &m_viewport);
}

void Renderer::drawFrame(float dt)
{
	for (auto pass : m_mainPasses)
	{
		pass->setup(*this);
		pass->draw(*this);
		pass->release(*this);
	}
	for (auto post : m_postProcesses)
	{
		post->setup(*this);
		post->draw(*this);
		post->release(*this);
	}

	m_swapChain->Present(1, 0);

}

void Renderer::endFrame()
{
	m_framesRendered++;
}

void Renderer::depthPrepass(const Camera& camera, Texture& tex)
{
    m_context->ClearDepthStencilView(tex.m_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ID3D11RenderTargetView* rtvs[] = { nullptr };
    m_context->OMSetRenderTargets(1, rtvs, tex.m_DSV);

    m_context->VSSetShader(m_depthPrepassShader.getVertexShader(), nullptr, 0);
    m_context->PSSetShader(nullptr, nullptr, 0);
    m_context->IASetInputLayout(m_depthPrepassShader.getInputLayout());

    m_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer* constants[] = { m_depthPrepassCB };
    m_context->VSSetConstantBuffers(0, 1, constants);

    for (auto& mesh : m_world.getObjects())
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
}

Shader Renderer::createShader(const std::string& fileName, const std::string& vsName, const std::string& psName, std::vector<D3D11_INPUT_ELEMENT_DESC> layout)
{
	std::ifstream t(fileName);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	if (str.empty())
	{
		return {};
	}

	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;

	ID3D10Blob* blob = nullptr;
	ID3D10Blob* blobErr = nullptr;

	if (!vsName.empty())
	{
		D3DCompile(str.c_str(), str.length(), nullptr, nullptr, nullptr, vsName.c_str(), "vs_5_0", 0, 0, &blob, &blobErr);
		if (blobErr)
		{
			std::string err = (char*)blobErr->GetBufferPointer();
			OutputDebugStringA(err.c_str());
			blobErr->Release();
			blobErr = nullptr;
		}

		m_device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader);

		D3D11_INPUT_ELEMENT_DESC desr[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		m_device->CreateInputLayout(layout.empty() ? desr : layout.data(), layout.empty() ? 3 : layout.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout);

		blob->Release();
		blob = nullptr;
	}
	
	if (!psName.empty())
	{
		D3DCompile(str.c_str(), str.length(), nullptr, nullptr, nullptr, psName.c_str(), "ps_5_0", 0, 0, &blob, &blobErr);
		if (blobErr)
		{
			std::string err = (char*)blobErr->GetBufferPointer();
			OutputDebugStringA(err.c_str());
			blobErr->Release();
		}

		m_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader);

		blob->Release();
		blob = nullptr;
	}

	t.close();

	Shader ret(inputLayout, vertexShader, pixelShader);
	if (inputLayout) inputLayout->Release();
	if (vertexShader) vertexShader->Release();
	if (pixelShader) pixelShader->Release();

	return ret;
}

void Renderer::addMainPass(Pass* p)
{
	m_mainPasses.push_back(p);
}

void Renderer::addPostPass(Pass* p)
{
	m_postProcesses.push_back(p);
}
