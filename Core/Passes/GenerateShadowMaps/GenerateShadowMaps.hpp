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
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;

private:
	Texture m_depthTexture;
};
