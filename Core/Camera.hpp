#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Camera
{
public:
	Camera()
	{
		m_projection = glm::mat4();
		m_view = glm::mat4();
	}

	void setProjection(float fov, float aspect, float zn, float zf)
	{
		m_ar = aspect;
		m_fov = fov;
		m_projection = glm::perspectiveLH_ZO(glm::radians(fov), aspect, zn, zf);
	}

	void setOrtho(float left, float right, float bottom, float top, float zn, float zf)
	{
		m_ar = (right - left) / (top - bottom); // ?
		m_projection = glm::orthoLH_ZO(left, right, bottom, top, zn, zf);
	}

	void setView(const glm::vec3& p, const glm::vec3& lootAt)
	{
		m_position = p;
		m_direction = glm::normalize(lootAt - p);
		m_view = glm::lookAtLH(p, lootAt, glm::vec3(0, 1, 0));
	}

	void setPosition(const glm::vec3& p) { m_position = p; }
	const glm::vec3& getPosition() const { return m_position; }
	glm::vec3& getPosition() { return m_position; }

	void setDirection(const glm::vec3& p) { m_direction = p; }
	const glm::vec3& getDirection() const { return m_direction; }

	glm::mat4& getProjection() { return m_projection; }
	glm::mat4& getView() { return m_view; }

    const glm::mat4& getProjection() const { return m_projection; }
    const glm::mat4& getView() const { return m_view; }

	void updateView()
	{
		m_view = glm::lookAtLH(m_position, m_position + m_direction, glm::vec3(0, 1, 0));
	}

	///
	void moveForward(float step)
	{
		m_position += m_direction * step;
	}

	void moveBackward(float step)
	{
		m_position -= m_direction * step;
	}

	void moveRight(float step)
	{
		m_position += glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), m_direction)) * step;
	}

	void moveLeft(float step)
	{
		m_position -= glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), m_direction)) * step;
	}

	void rotate(float pitch, float yaw)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		m_direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		m_direction.y = sin(glm::radians(pitch));
		m_direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		m_direction = -glm::normalize(m_direction);
		m_pitch = pitch;
		m_yaw = yaw;
	}

	float getAR() const { return m_ar; }
	float getFov() const { return m_fov; }
	float getPitch() const { return m_pitch; }
	float getYaw() const { return m_yaw; }

private:
	glm::mat4 m_projection;
	glm::mat4 m_view;
	glm::vec3 m_position;
	glm::vec3 m_direction;
	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
	float m_ar = 1.0f;
	float m_fov = 0.0f;
};