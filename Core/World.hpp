#pragma once

#include <string>
#include <vector>
#include "Camera.hpp"

#include <d3d11.h>

#include "tiny_obj_loader.h"
#include "Light.hpp"
#include <map>

class Renderer;

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
	};

	World() {}

	std::vector<Mesh>::iterator loadObjects(const std::string& fileName, const std::string& materialBaseDir, Renderer& renderer);
	void deinitializeBuffers();

	World(const World& rhs) = delete;
	World(World&& rhs) = delete;
	World& operator=(const World& rhs) = delete;
	World& operator=(World&& rhs) = delete;

	void setCamera(Camera cam) { m_camera = cam; }
	Camera& getCamera() { return m_camera; }

	const std::vector<Mesh>& getObjects() const { return m_objects; }
	std::vector<Mesh>& getObjects() { return m_objects; }

	void addLight(Light l) { m_lights.push_back(l); }
	std::vector<Light>& getLights() { return m_lights; }
	const std::vector<Light>& getLights() const { return m_lights; }
	glm::vec3 getAmbientLight() const { return m_ambientLight; }
	void setAmbientLight(const glm::vec3& l) { m_ambientLight = l; }

private:
	void initializeBuffers(Renderer& renderer);

	Camera m_camera;
	tinyobj::attrib_t m_attrib;
	std::vector<tinyobj::shape_t> m_shapes;
	std::vector<tinyobj::material_t> m_materials;
	std::vector<Mesh> m_objects;
	std::vector<Light> m_lights;
	glm::vec3 m_ambientLight = {1.0f, 1.0f, 1.0f};
	std::map<std::string, ID3D11ShaderResourceView*> m_textures;
	std::string m_materialBaseDir;
};