#pragma once

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

class Renderer;
class Resources;

class GBuffer
{
public:
	GBuffer(Renderer& renderer, Resources& resources);
	~GBuffer();

	void bindForWriting(Renderer& renderer);
	void clear(Renderer& renderer);

	ID3D11Texture2D* m_diffuseTexture = nullptr;
	ID3D11Texture2D* m_NormalTexture = nullptr;
	ID3D11Texture2D* m_PositionTexture = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;

	ID3D11ShaderResourceView* m_diffuseSRV = nullptr;
	ID3D11ShaderResourceView* m_positionSRV = nullptr;
	ID3D11ShaderResourceView* m_normalSRV = nullptr;

	ID3D11RenderTargetView* m_diffuseRT = nullptr;
	ID3D11RenderTargetView* m_positionRT = nullptr;
	ID3D11RenderTargetView* m_normalRT = nullptr;

	ID3D11DepthStencilView* m_depthStencilView = nullptr;
};