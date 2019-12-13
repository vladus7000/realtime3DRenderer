#pragma once

#include <string>
#include <vector>
#include "Camera.hpp"
#include "Texture.hpp"
#include <d3d11.h>

#include "tiny_obj_loader.h"
#include "Light.hpp"
#include <map>
#include <glm/gtx/compatibility.hpp>

class Resources;

class World
{
public:
	struct Mesh
	{
		ID3D11Buffer* vert_vb;
		ID3D11Buffer* norm_vb;
		ID3D11Buffer* tcoords_vb;
		ID3D11Buffer* indexBuffer = nullptr;

		//Separate to material
		ID3D11ShaderResourceView* albedo = nullptr;
		ID3D11ShaderResourceView* normal = nullptr;
		int numIndices = 0;
		std::string name;
		glm::mat4 worldMatrix = glm::mat4(1.0f);
		glm::vec3 minCoord = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		glm::vec3 maxCoord = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
	};

	World();
	~World();

	void initSun(Resources& resources);

	std::vector<Mesh>::iterator loadObjects(const std::string& fileName, const std::string& materialBaseDir, Resources& resources);

	World(const World& rhs) = delete;
	World(World&& rhs) = delete;
	World& operator=(const World& rhs) = delete;
	World& operator=(World&& rhs) = delete;

	void setCamera(Camera cam) { m_camera = cam; }
	Camera& getCamera() { return m_camera; }

	const std::vector<Mesh>& getObjects() const { return m_objects; }
	std::vector<Mesh>& getObjects() { return m_objects; }

	void addLight(Light l);
	std::vector<Light>& getLights() { return m_lights; }
	const std::vector<Light>& getLights() const { return m_lights; }

	bool getIsDay() { return m_isDay; }

	void setSunAngle(float angle) { m_sunAngle = angle; }
	float getSunAngle() { return m_sunAngle; }

	void updateSun(float dt);

private:
	void initializeBuffers(Resources& resources);
	void deinitializeBuffers();

private:
	Camera m_camera;
	tinyobj::attrib_t m_attrib;
	std::vector<tinyobj::shape_t> m_shapes;
	std::vector<tinyobj::material_t> m_materials;
	std::vector<Mesh> m_objects;
	std::vector<Light> m_lights;

	std::map<std::string, Texture> m_textures;
	std::string m_materialBaseDir;

	Mesh* m_sunObject;
	Light* m_sunLight;
	bool m_isDay = true;
	float m_sunAngle = 0.0f;
	glm::vec3 m_sunColorMorning;
	glm::vec3 m_sunColorDay;
};