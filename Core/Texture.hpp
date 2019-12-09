#pragma once
#include <d3d11.h>

class Texture
{
public:
	void release()
	{
		if (m_texture)
		{
			m_texture->Release();
		}
		if (m_SRV)
		{
			m_SRV->Release();
		}
		if (m_RT)
		{
			m_RT->Release();
		}
		if (m_DSV)
		{
			m_DSV->Release();
		}
	}

	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;
	ID3D11RenderTargetView* m_RT = nullptr;
	ID3D11DepthStencilView* m_DSV = nullptr;
};