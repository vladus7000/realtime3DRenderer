#pragma once

class Renderer;

class Pass
{
public:
	Pass() {}
	virtual ~Pass() {}

	virtual void setup(Renderer& renderer) = 0;
	virtual void release(Renderer& renderer) = 0;
	virtual void draw(Renderer& renderer) = 0;

	Pass(const Pass& rhs) = delete;
	Pass(Pass&& rhs) = delete;
	Pass& operator=(const Pass& rhs) = delete;
	Pass& operator=(Pass&& rhs) = delete;
};