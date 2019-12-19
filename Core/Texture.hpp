#pragma once
#include <d3d11.h>
#include <wrl.h>

class Texture
{
public:
    ~Texture() = default;
 
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RT;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DSV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_UAV;
	int m_w = 0;
	int m_h = 0;
};