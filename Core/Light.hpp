#pragma once

#include "Camera.hpp"

class Light
{
public:
	enum class Type
	{
		Directional,
		Point,
		Spot
	};

	Light() {}
	
	void updateMatrices()
	{
		m_camera.setPosition(m_position);
		m_camera.setDirection(m_direction);
		m_camera.updateView();
		if (perspective)
		{
			m_camera.setProjection(60.0f, m_aspect, 0.1f, 1000.0f);
		}
		else
		{
			m_camera.setOrtho(-60.0f, 60.f, -50.0f, 80.0f, 0.1f, 1000.0f);
		}
	}

	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_attenuation;
	glm::vec3 m_intensity;
	Type m_type = Type::Directional;
	float m_cutoff;
	bool perspective = true;
	float m_aspect = 800.0f / 600.0f;
	Camera m_camera;
};