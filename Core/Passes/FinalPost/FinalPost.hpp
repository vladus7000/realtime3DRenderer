#pragma once

#include "Pass.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class FinalPost : public Pass
{
public:
	FinalPost() {}
private:
	~FinalPost();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;

private:
	Shader m_mainShader;
};
