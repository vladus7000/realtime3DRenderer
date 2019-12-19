#pragma once
#include "Settings.hpp"

class RenderSettings : public Settings
{
public:
	RenderSettings() {}

	bool useCSforLighting = false;
};