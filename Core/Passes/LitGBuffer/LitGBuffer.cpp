#include "LitGBuffer.hpp"
#include "Renderer.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Light.hpp"

LitGBuffer::~LitGBuffer()
{
	if (m_constantBuffer)
	{
		m_constantBuffer->Release();
	}

	m_sampler->Release();
	m_depthState->Release();
	m_blendState->Release();
}

void LitGBuffer::setup(Renderer& renderer)
{
	auto context = renderer.getContext();
	if (!m_constantBuffer)
	{
		auto device = renderer.getDevice();
		D3D11_BUFFER_DESC buffDesc;
		buffDesc.ByteWidth = sizeof(float[4]) * 4 + sizeof(float[16]); // must be multiply of 16
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;
		device->CreateBuffer(&buffDesc, nullptr, &m_constantBuffer);

		m_mainShader = renderer.createShader("shaders/litGBuffer/litGBuffer.hlsl", "vsmain", "psmain");
		m_toneShader = renderer.createShader("shaders/litGBuffer/tonemapGBuffer.hlsl", "vsmain", "psmain");
		D3D11_SAMPLER_DESC sampler;
		ZeroMemory(&sampler, sizeof(D3D11_SAMPLER_DESC));
		sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		device->CreateSamplerState(&sampler, &m_sampler);

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
		
		device->CreateDepthStencilState(&dsDesc, &m_depthState);

		D3D11_BLEND_DESC blend;
		ZeroMemory(&blend, sizeof(D3D11_BLEND_DESC));
		blend.AlphaToCoverageEnable = false;
		blend.IndependentBlendEnable = false;
		blend.RenderTarget[0].BlendEnable = true;
		blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

		blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		device->CreateBlendState(&blend, &m_blendState);
	}
}

void LitGBuffer::release(Renderer& renderer)
{
	auto context = renderer.getContext();

	ID3D11RenderTargetView* rtvs[] = { nullptr };
	context->OMSetRenderTargets(1, rtvs, nullptr);
	ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr, nullptr };
	context->PSSetShaderResources(0, 3, srvs);
	context->PSSetShader(nullptr, nullptr, 0);
	context->VSSetShader(nullptr, nullptr, 0);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
	context->OMSetDepthStencilState(nullptr, 0);
	context->OMSetBlendState( nullptr, nullptr, 0xffffffff);
	ID3D11SamplerState* samplers[] = { nullptr };
	context->PSSetSamplers(0, 1, samplers);
}

void LitGBuffer::draw(Renderer& renderer)
{
	auto context = renderer.getContext();
	auto& world = renderer.getWorld();

	auto& gbuffer = renderer.getGBuffer();
	Texture* shadowMap = renderer.getTextureResource(Renderer::TextureResouces::ShadowMap);

	{

		auto context = renderer.getContext();
		float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->ClearRenderTargetView(renderer.getHDRTexture().m_RT, color);

		ID3D11RenderTargetView* rtvs[] = { renderer.getHDRTexture().m_RT };

		context->OMSetRenderTargets(1, rtvs, gbuffer.m_depthStencilView);
		ID3D11ShaderResourceView* srvs[] = { gbuffer.m_diffuseSRV, gbuffer.m_normalSRV, gbuffer.m_positionSRV, shadowMap? shadowMap->m_SRV:nullptr };
		context->PSSetShaderResources(0, shadowMap ? 4 : 3 , srvs);
	}
	context->PSSetSamplers(0, 1, &m_sampler);

	context->OMSetDepthStencilState(m_depthState, 0);

	context->RSSetViewports(1, &renderer.getMainViewport());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(nullptr);
	context->PSSetShader(m_mainShader.getPixelShader(), nullptr, 0);
	context->VSSetShader(m_mainShader.getVertexShader(), nullptr, 0);

	context->OMSetBlendState(m_blendState, nullptr, 0xffffffff);

	glm::vec3 ambient = renderer.getWorld().getAmbientLight();

	for (auto& l : renderer.getWorld().getLights())
	{
		D3D11_MAPPED_SUBRESOURCE res;
		context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		struct Data
		{
			float ambient[4];
			float pos_type[4];
			float dir[4];
			float intensity[4];
			float sunViewProjection[16];
		};
		Data* buffer = reinterpret_cast<Data*>(res.pData);

		memcpy(buffer->ambient, &ambient[0], sizeof(float[3]));
		memcpy(buffer->pos_type, &l.m_position[0], sizeof(float[3]));
		buffer->pos_type[3] = l.m_type == Light::Type::Directional ? 0.0f : 1.0f;

		memcpy(buffer->dir, &l.m_direction[0], sizeof(float[3]));
		memcpy(buffer->intensity, &l.m_intensity[0], sizeof(float[3]));

		glm::mat4 mvp = l.m_camera.getProjection() * l.m_camera.getView();

		memcpy(buffer->sunViewProjection, &mvp[0][0], sizeof(float[16]));
		context->Unmap(m_constantBuffer, 0);

		ID3D11Buffer* constants[] = { m_constantBuffer };
		context->PSSetConstantBuffers(0, 1, constants);
		context->VSSetConstantBuffers(0, 1, constants);

		context->Draw(4, 0);
	}

	{
		ID3D11RenderTargetView* rtvs[] = { renderer.getDisplayBB() };

		context->OMSetRenderTargets(1, rtvs, gbuffer.m_depthStencilView);
		ID3D11ShaderResourceView* srvs[] = { renderer.getHDRTexture().m_SRV, gbuffer.m_diffuseSRV };
		context->PSSetShaderResources(0, 2, srvs);
	}

	context->PSSetShader(m_toneShader.getPixelShader(), nullptr, 0);
	context->VSSetShader(m_toneShader.getVertexShader(), nullptr, 0);

	context->Draw(4, 0);
}

