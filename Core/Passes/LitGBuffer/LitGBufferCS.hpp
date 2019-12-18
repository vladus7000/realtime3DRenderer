#pragma once

#include "Pass.hpp"
#include "Shader.hpp"

struct ID3D11Buffer;
struct ID3D11SamplerState;
class Texture;
class LitGBufferCS : public Pass
{
public:
	LitGBufferCS() {}
private:
	~LitGBufferCS();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void draw(Renderer& renderer) override;

private:
	ID3D11Buffer* m_lightsCB = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;
	Shader m_csShader;
	Texture* m_shadowMap;
	Texture* m_cubeMap;
	static const int m_lightMaxNumber = 50;
};
