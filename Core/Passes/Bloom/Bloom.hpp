#pragma once

#include "Pass.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class Bloom : public Pass
{
public:
	Bloom() {}
private:
	~Bloom();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;

private:
	Shader m_mainShader;
};
