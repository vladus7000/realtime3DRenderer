#pragma once

#include "Pass.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class MainZpass : public Pass
{
public:
	MainZpass() {}
private:
	~MainZpass();
	virtual void setup(Renderer& renderer, Resources& resources) override;
	virtual void release(Renderer& renderer, Resources& resources) override;
	virtual void execute(Renderer& renderer) override;
};
