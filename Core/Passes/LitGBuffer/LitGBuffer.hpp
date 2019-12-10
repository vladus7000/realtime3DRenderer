#pragma once

#include "Pass.hpp"
#include "Shader.hpp"

struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
class LitGBuffer : public Pass
{
public:
	LitGBuffer() {}
private:
	~LitGBuffer();
	virtual void setup(Renderer& renderer) override;
	virtual void release(Renderer& renderer) override;
	virtual void draw(Renderer& renderer) override;

private:
	ID3D11Buffer* m_constantBuffer = nullptr;
	ID3D11SamplerState* m_sampler = nullptr;
	ID3D11DepthStencilState* m_depthState;
	ID3D11BlendState* m_blendState = nullptr;
	Shader m_mainShader;
	Shader m_toneShader;
};
