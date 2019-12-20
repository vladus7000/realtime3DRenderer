#pragma once
#include "Settings.hpp"
class RenderSettings : public Settings
{
public:
	RenderSettings()
	{

		cascadesEnd[0] = 0.1f;
		cascadesEnd[1] = 50.0f;
		cascadesEnd[2] = 200.0f;
		cascadesEnd[3] = 1000.0f;
	}

	bool useCSforLighting = false;
	//CSM settings
	static const unsigned int cascadesCount = 3;
	float cascadesEnd[cascadesCount + 1];
	float cascadesEndClip[cascadesCount];
	glm::mat4 cascadesProjectionMatrix[cascadesCount];
};