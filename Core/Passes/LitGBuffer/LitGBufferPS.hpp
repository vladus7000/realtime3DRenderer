#pragma once

#include "Pass.hpp"
#include "Shader.hpp"

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
	virtual void draw(Renderer& renderer) override;

private:
	ID3D11Buffer* m_lightsCB = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;
	ID3D11DepthStencilState* m_depthState;
	ID3D11BlendState* m_blendState = nullptr;
	Shader m_mainShader;

	Texture* m_shadowMap;
	Texture* m_cubeMap;
	static const int m_lightMaxNumber = 50;
};
