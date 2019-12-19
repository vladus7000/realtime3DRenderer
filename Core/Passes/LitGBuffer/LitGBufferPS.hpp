#pragma once

#include "Pass.hpp"
#include "Shader.hpp"
#include "ConstantBuffer.hpp"

struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
class Texture;
class LitGBufferPS : public Pass
{
public:
	LitGBufferPS() {}
private:
	~LitGBufferPS();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;

private:
	static const int m_lightMaxNumber = 50;
	struct LightCB
	{
		float pos_type[4 * LitGBufferPS::m_lightMaxNumber];
		float dir[4 * LitGBufferPS::m_lightMaxNumber];
		float intensity[4 * LitGBufferPS::m_lightMaxNumber];
		float sunViewProjection[16 * 3];
		float viewMatrix[16];
		float projMatrix[16];
		float viewPosition[4];
		float numLights[4];
	};
	ConstantBuffer<LightCB> m_lightsCB;
	ID3D11SamplerState* m_sampler = nullptr;
	ID3D11DepthStencilState* m_depthState;
	ID3D11BlendState* m_blendState = nullptr;
	Shader m_mainShader;

	Texture* m_shadowMapC1;
	Texture* m_shadowMapC2;
	Texture* m_shadowMapC3;
	Texture* m_cubeMap;
};
