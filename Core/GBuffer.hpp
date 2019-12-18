#pragma once

#include "Texture.hpp"
//struct ID3D11Texture2D;
//struct ID3D11ShaderResourceView;
//struct ID3D11RenderTargetView;
//struct ID3D11DepthStencilView;

class Renderer;
class Resources;

class GBuffer
{
public:
	GBuffer(Renderer& renderer, Resources& resources);
	~GBuffer();

	void bindForWriting(Renderer& renderer);
	void clearColor(Renderer& renderer);
	void clearDepth(Renderer& renderer);

	Texture m_diffuse;
	Texture m_normal_metalnes;
	Texture m_position_rough;
	Texture m_depth;
};