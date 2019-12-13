#include "GBuffer.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "Resources.hpp"

GBuffer::GBuffer(Renderer& renderer, Resources& resources)
{
	auto device = resources.getDevice();
	int width = renderer.getWindow().getWidth();
	int height = renderer.getWindow().getHeight();

	{ //depth stencil
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		device->CreateTexture2D(&desc, nullptr, &m_depthStencilTexture);
		device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilView);
	}

	{//diffuse
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		device->CreateTexture2D(&desc, nullptr, &m_diffuseTexture);
		device->CreateShaderResourceView(m_diffuseTexture, nullptr, &m_diffuseSRV);
		device->CreateRenderTargetView(m_diffuseTexture, nullptr, &m_diffuseRT);
	}

	{//positions
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		device->CreateTexture2D(&desc, nullptr, &m_PositionTexture);
		device->CreateShaderResourceView(m_PositionTexture, nullptr, &m_positionSRV);
		device->CreateRenderTargetView(m_PositionTexture, nullptr, &m_positionRT);
	}

	{//normals
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		device->CreateTexture2D(&desc, nullptr, &m_NormalTexture);
		device->CreateShaderResourceView(m_NormalTexture, nullptr, &m_normalSRV);
		device->CreateRenderTargetView(m_NormalTexture, nullptr, &m_normalRT);
	}
}

GBuffer::~GBuffer()
{
	m_diffuseTexture->Release();
	m_NormalTexture->Release();
	m_PositionTexture->Release();
	m_depthStencilTexture->Release();

	m_diffuseSRV->Release();
	m_positionSRV->Release();
	m_normalSRV->Release();

	m_diffuseRT->Release();
	m_positionRT->Release();
	m_normalRT->Release();

	m_depthStencilView->Release();
}

void GBuffer::bindForWriting(Renderer& renderer)
{
	auto context = renderer.getContext();

	ID3D11RenderTargetView* rtvs[] = { m_diffuseRT , m_positionRT , m_normalRT };;
	context->OMSetRenderTargets(3, rtvs, m_depthStencilView);
}

void GBuffer::clear(Renderer& renderer)
{
	auto context = renderer.getContext();
	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(m_diffuseRT, color);
	context->ClearRenderTargetView(m_positionRT, color);
	context->ClearRenderTargetView(m_normalRT, color);
	context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
