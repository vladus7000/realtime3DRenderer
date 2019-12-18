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
		device->CreateTexture2D(&desc, nullptr, m_depth.m_texture.GetAddressOf());
		device->CreateDepthStencilView(m_depth.m_texture.Get(), nullptr, m_depth.m_DSV.GetAddressOf());
		m_depth.m_w = width;
		m_depth.m_h = height;
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
		device->CreateTexture2D(&desc, nullptr, m_diffuse.m_texture.GetAddressOf());
		device->CreateShaderResourceView(m_diffuse.m_texture.Get(), nullptr, m_diffuse.m_SRV.GetAddressOf());
		device->CreateRenderTargetView(m_diffuse.m_texture.Get(), nullptr, m_diffuse.m_RT.GetAddressOf());
		m_diffuse.m_w = width;
		m_diffuse.m_h = height;
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
		device->CreateTexture2D(&desc, nullptr, m_position_rough.m_texture.GetAddressOf());
		device->CreateShaderResourceView(m_position_rough.m_texture.Get(), nullptr, m_position_rough.m_SRV.GetAddressOf());
		device->CreateRenderTargetView(m_position_rough.m_texture.Get(), nullptr, m_position_rough.m_RT.GetAddressOf());
		m_position_rough.m_w = width;
		m_position_rough.m_h = height;
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
		device->CreateTexture2D(&desc, nullptr, m_normal_metalnes.m_texture.GetAddressOf());
		device->CreateShaderResourceView(m_normal_metalnes.m_texture.Get(), nullptr, m_normal_metalnes.m_SRV.GetAddressOf());
		device->CreateRenderTargetView(m_normal_metalnes.m_texture.Get(), nullptr, m_normal_metalnes.m_RT.GetAddressOf());
		m_normal_metalnes.m_w = width;
		m_normal_metalnes.m_h = height;
	}
}

GBuffer::~GBuffer()
{
}

void GBuffer::bindForWriting(Renderer& renderer)
{
	auto context = renderer.getContext();

	ID3D11RenderTargetView* rtvs[] = { m_diffuse.m_RT.Get() , m_position_rough.m_RT.Get() , m_normal_metalnes.m_RT.Get() };;
	context->OMSetRenderTargets(3, rtvs, m_depth.m_DSV.Get());
}

void GBuffer::clearDepth(Renderer& renderer)
{
	auto context = renderer.getContext();
	context->ClearDepthStencilView(m_depth.m_DSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void GBuffer::clearColor(Renderer& renderer)
{
	auto context = renderer.getContext();
	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(m_diffuse.m_RT.Get(), color);
	context->ClearRenderTargetView(m_position_rough.m_RT.Get(), color);
	context->ClearRenderTargetView(m_normal_metalnes.m_RT.Get(), color);
}
