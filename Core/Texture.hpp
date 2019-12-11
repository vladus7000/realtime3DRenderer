#pragma once
#include <d3d11.h>

class Renderer;

class Texture
{
public:
    ~Texture();
    void createDepthStencilTexture(int w, int h, Renderer& renderer);

	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;
	ID3D11RenderTargetView* m_RT = nullptr;
	ID3D11DepthStencilView* m_DSV = nullptr;
	int m_w = 0;
	int m_h = 0;
private:
    void release();
};