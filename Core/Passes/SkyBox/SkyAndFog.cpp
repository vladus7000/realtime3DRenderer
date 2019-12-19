#include "SkyAndFog.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "World.hpp"
#include "GBuffer.hpp"
#include "Resources.hpp"

SkyAndFog::~SkyAndFog()
{
	m_skyBox->Release();
	m_constantBuffer->Release();
	//m_depthState->Release();
}

void SkyAndFog::setup(Renderer& renderer, Resources& resources)
{
	auto context = renderer.getContext();
	auto device = resources.getDevice();
	m_dayCubeMap = resources.getResource<Texture>(Resources::ResoucesID::EnvCubeMapDay);
	m_nightCubeMap = resources.getResource<Texture>(Resources::ResoucesID::EnvCubeMapNight);

	if (!m_skyBox)
	{
		struct Vertex
		{
			glm::vec4 pos;
		};

		Vertex pVertex[4];
		// Map texels to pixels
		float w = (float)renderer.getWindow().getWidth();
		float h = (float)renderer.getWindow().getHeight();
		float fHighW = 1.0f - (1.0f / w);
		float fLowW = -1.0f + (1.0f / w);

		float fHighH = 1.0f - (1.0f / h);
		float fLowH = -1.0f + (1.0f / h);

		pVertex[0].pos = glm::vec4(fLowW, fLowH, 1.0f, 1.0f);
		pVertex[1].pos = glm::vec4(fLowW, fHighH, 1.0f, 1.0f);
		pVertex[2].pos = glm::vec4(fHighW, fLowH, 1.0f, 1.0f);
		pVertex[3].pos = glm::vec4(fHighW, fHighH, 1.0f, 1.0f);

		UINT uiVertBufSize = 4 * sizeof(Vertex);
		//Vertex Buffer
		{
			D3D11_BUFFER_DESC vbdesc;
			vbdesc.ByteWidth = uiVertBufSize;
			vbdesc.Usage = D3D11_USAGE_IMMUTABLE;
			vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbdesc.CPUAccessFlags = 0;
			vbdesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pVertex;
			device->CreateBuffer(&vbdesc, &InitData, &m_skyBox);
		}
		{
			D3D11_BUFFER_DESC cbuffer;
			cbuffer.ByteWidth = sizeof(float[16]) + sizeof(float[4]);
			cbuffer.Usage = D3D11_USAGE_DYNAMIC;
			cbuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbuffer.MiscFlags = 0;

			device->CreateBuffer(&cbuffer, nullptr, &m_constantBuffer);
		}

		m_depthState = resources.getResource<ID3D11DepthStencilState>(Resources::ResoucesID::EqualDepthState);
		m_mainShader = resources.createShader("shaders/skyBox/skyBox.hlsl", "vsmain", "psmain");
	}
}

void SkyAndFog::release(Renderer& renderer, Resources& resources)
{
	auto context = renderer.getContext();
	UINT uStrides = sizeof(float[4]);
	UINT uOffsets = 0;
	ID3D11Buffer* pBuffers[1] = { nullptr };

	context->IASetVertexBuffers(0, 1, pBuffers, &uStrides, &uOffsets);

	ID3D11RenderTargetView* rtvs[] = { nullptr };

	auto& gbuffer = renderer.getGBuffer();
	context->OMSetRenderTargets(1, rtvs, nullptr);
	ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr };
	context->PSSetShaderResources(0, 2, srvs);
	context->VSSetConstantBuffers(0, 1, pBuffers);
	context->PSSetConstantBuffers(0, 1, pBuffers);
	context->OMSetDepthStencilState(nullptr, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);

	ID3D11SamplerState* samplers[] = { nullptr };
	context->PSSetSamplers(0, 1, samplers);

	context->IASetInputLayout(nullptr);
}

void SkyAndFog::execute(Renderer& renderer)
{
	auto context = renderer.getContext();
	UINT uStrides = sizeof(float[4]);
	UINT uOffsets = 0;
	ID3D11Buffer* pBuffers[1] = { m_skyBox };

	context->IASetVertexBuffers(0, 1, pBuffers, &uStrides, &uOffsets);
	//context->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->PSSetShader(m_mainShader.getPixelShader(), nullptr, 0);
	context->VSSetShader(m_mainShader.getVertexShader(), nullptr, 0);
	context->IASetInputLayout(m_mainShader.getInputLayout());

	D3D11_MAPPED_SUBRESOURCE res;
	context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	struct Data
	{
		float viewProjection[16];
		float sunAngle[4];
	};
	Data* buffer = reinterpret_cast<Data*>(res.pData);
	auto& camera = renderer.getWorld().getCamera();
	glm::mat4 vp = /* camera.getProjection() * */  glm::inverse(camera.getView());

	memcpy(buffer->viewProjection, &vp[0][0], sizeof(float[16]));
	buffer->sunAngle[0] = renderer.getWorld().getSunAngle();
	buffer->sunAngle[1] = 0.0f;
	buffer->sunAngle[2] = 0.0f;
	buffer->sunAngle[3] = 0.0f;
	context->Unmap(m_constantBuffer, 0);

	ID3D11RenderTargetView* rtvs[] = { renderer.getDisplayBB() };

	auto& gbuffer = renderer.getGBuffer();
	context->OMSetRenderTargets(1, rtvs, gbuffer.m_depth.m_DSV.Get());
	ID3D11ShaderResourceView* srvs[] = { m_dayCubeMap->m_SRV.Get(), m_nightCubeMap->m_SRV.Get() };
	context->PSSetShaderResources(0, 2, srvs);
	ID3D11SamplerState* samplers[] = { m_dayCubeMap->m_sampler.Get() };
	context->PSSetSamplers(0, 1, samplers);

	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->OMSetDepthStencilState(m_depthState, 0);
	context->Draw(4, 0);
}

