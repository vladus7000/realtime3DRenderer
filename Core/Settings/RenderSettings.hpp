#pragma once
#include "Settings.hpp"
class RenderSettings : public Settings
{
public:
	RenderSettings()
	{
		cascadesEnd = new float[cascadesCount + 1];
		cascadesEndClip = new float[cascadesCount];

		float cascadeEnd[] = { 0.1f, 50.0f, 200.0f, 1000.0f };
		cascadesEnd[0] = 0.1f;
		cascadesEnd[1] = 50.0f;
		cascadesEnd[2] = 200.0f;
		cascadesEnd[3] = 1000.0f;

		cascadesProjectionMatrix = new glm::mat4[cascadesCount];

	}
	~RenderSettings()
	{
		delete[] cascadesEnd;
		delete[] cascadesProjectionMatrix;
		delete[] cascadesEndClip;
	}
	bool useCSforLighting = false;
	//CSM settings
	unsigned int cascadesCount = 3;
	float* cascadesEnd;
	float* cascadesEndClip;
	glm::mat4* cascadesProjectionMatrix;
};