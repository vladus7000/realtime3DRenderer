#pragma once

#include "Pass.hpp"
#include "Shader.hpp"

struct ID3D11Buffer;
struct ID3D11SamplerState;
class GenerateGBuffer : public Pass
{
public:
	GenerateGBuffer() {}
private:
	~GenerateGBuffer();
	virtual void setup(Renderer& renderer) override;
	virtual void release(Renderer& renderer) override;
	virtual void draw(Renderer& renderer) override;

private:
	ID3D11Buffer* m_constantBuffer = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;
	Shader m_mainShader;
};
