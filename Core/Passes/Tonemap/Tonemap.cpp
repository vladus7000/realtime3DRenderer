#include "Tonemap.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Resources.hpp"

Tonemap::~Tonemap()
{
	m_sampler->Release();
	m_depthState->Release();
}

void Tonemap::setup(Renderer& renderer, Resources& resources)
{
	if (!m_mainShader.getPixelShader())
	{
		m_mainShader = resources.createShader("shaders/tonemap/tonemap.hlsl", "vsmain", "psmain");

		D3D11_SAMPLER_DESC sampler;
		ZeroMemory(&sampler, sizeof(D3D11_SAMPLER_DESC));
		sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		resources.getDevice()->CreateSamplerState(&sampler, &m_sampler);

		D3D11_DEPTH_STENCIL_DESC dsDesc;

		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;

		// Stencil test parameters
		dsDesc.StencilEnable = false;
		dsDesc.StencilReadMask = 0xFF;
		dsDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create depth stencil state

		resources.getDevice()->CreateDepthStencilState(&dsDesc, &m_depthState);
	}
}

void Tonemap::release(Renderer& renderer, Resources& resources)
{
	ID3D11SamplerState* samplers[] = { nullptr };
	renderer.getContext()->PSSetSamplers(0, 1, samplers);
	renderer.getContext()->OMSetDepthStencilState(nullptr, 0);
}

void Tonemap::draw(Renderer& renderer)
{
	auto context = renderer.getContext();
	auto& gbuffer = renderer.getGBuffer();
	{
		ID3D11RenderTargetView* rtvs[] = { renderer.getDisplayBB() };

		context->OMSetRenderTargets(1, rtvs, gbuffer.m_depthStencilView);
		ID3D11ShaderResourceView* srvs[] = { renderer.getHDRTexture().m_SRV.Get() };
		context->PSSetShaderResources(0, 1, srvs);
	}
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(nullptr);
	context->OMSetDepthStencilState(m_depthState, 0);
	context->PSSetShader(m_mainShader.getPixelShader(), nullptr, 0);
	context->VSSetShader(m_mainShader.getVertexShader(), nullptr, 0);
	ID3D11SamplerState* samplers[] = { m_sampler };
	context->PSSetSamplers(0, 1, samplers);
	context->Draw(4, 0);
}
