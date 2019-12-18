#pragma once

#include "Pass.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class SSR : public Pass
{
public:
	SSR() {}
private:
	~SSR();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;

private:
	Shader m_mainShader;
};
