#pragma once

#include "Pass.hpp"
#include "Shader.hpp"

struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
class GenerateGBuffer : public Pass
{
public:
	GenerateGBuffer() {}
private:
	~GenerateGBuffer();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;

private:
	ID3D11Buffer* m_constantBuffer = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;
	ID3D11DepthStencilState* m_depthState;
	Shader m_mainShader;
};
