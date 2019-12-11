#pragma once

#include "Pass.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

struct ID3D11Buffer;
struct ID3D11SamplerState;

class GenerateShadowMaps : public Pass
{
public:
	GenerateShadowMaps() {}
private:
	~GenerateShadowMaps();
	virtual void setup(Renderer& renderer) override;
	virtual void release(Renderer& renderer) override;
	virtual void draw(Renderer& renderer) override;

private:
	ID3D11Buffer* m_constantBuffer = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;
	Shader m_mainShader;
	Texture m_depthTexture;
};
