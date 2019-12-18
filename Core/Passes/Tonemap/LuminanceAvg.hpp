#pragma once

#include "Pass.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;

class LuminanceAvg : public Pass
{
public:
	LuminanceAvg() {}
private:
	~LuminanceAvg();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;

private:
	ID3D11Buffer* m_constantBuffer = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;
	ID3D11DepthStencilState* m_depthState = nullptr;
	Shader m_mainShader;
};
