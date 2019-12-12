#include "Texture.hpp"
#include "Renderer.hpp"

#include <d3d11.h>

void Texture::release()
{
    if (m_texture)
    {
        m_texture->Release();
        m_texture = nullptr;
    }
    if (m_SRV)
    {
        m_SRV->Release();
        m_SRV = nullptr;
    }
    if (m_RT)
    {
        m_RT->Release();
        m_RT = nullptr;
    }
    if (m_DSV)
    {
        m_DSV->Release();
        m_DSV = nullptr;
    }
	if (m_sampler)
	{
		m_sampler->Release();
		m_sampler = nullptr;
	}
}

Texture::~Texture()
{
    release();
}

void Texture::createDepthStencilTexture(int w, int h, Renderer& renderer)
{
    release();
    auto device = renderer.getDevice();
	m_w = w;
	m_h = h;
    { //depth stencil
        D3D11_TEXTURE2D_DESC desc;
        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.Format = DXGI_FORMAT_R32_TYPELESS;//DXGI_FORMAT_D32_FLOAT_S8X24_UINT
        desc.Width = w;
        desc.Height = h;
        desc.MipLevels = 1;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        device->CreateTexture2D(&desc, nullptr, &m_texture);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags = 0;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        device->CreateDepthStencilView(m_texture, &dsvDesc, &m_DSV);
        device->CreateShaderResourceView(m_texture, &srvDesc, &m_SRV);
    }
}
