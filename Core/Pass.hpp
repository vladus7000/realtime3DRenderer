#pragma once

class Renderer;
class Resources;

class Pass
{
public:
	Pass() {}
	virtual ~Pass() {}

	virtual void setup(Renderer& renderer, Resources& resources) = 0;
	virtual void release(Renderer& renderer, Resources& resources) = 0;
	virtual void execute(Renderer& renderer) = 0;

	Pass(const Pass& rhs) = delete;
	Pass(Pass&& rhs) = delete;
	Pass& operator=(const Pass& rhs) = delete;
	Pass& operator=(Pass&& rhs) = delete;
};