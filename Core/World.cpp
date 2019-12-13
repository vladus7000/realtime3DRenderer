#include "World.hpp"
#include "Resources.hpp"

//#include <D3DX11tex.h>
#undef min
#undef max

#include <algorithm>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

World::World()
{
}

World::~World()
{
	deinitializeBuffers();
}

void World::initSun(Resources& resources)
{
	auto it = loadObjects("cube.obj", "", resources);
	m_sunObject = &*it;
	m_sunObject->name = "sunObject_";
	m_sunColorMorning = { 243.0f / 255.0f, 60.0f / 255.0f, 10.0f / 255.0f };
	m_sunColorDay = { 252.0f / 255.0f, 212.0f / 255.0f, 64.0f / 255.0f };
	Light l;
	l.m_direction = -glm::vec3{ 90.0f, 25.0f, 0.0f };
	l.m_intensity = m_sunColorDay; // Sun
	l.m_intensity *= 5.0f;
	l.m_position = glm::vec3{ 500.0f, 25.0f, 0.0f };
	l.m_type = Light::Type::Directional;
	l.perspective = false;
	l.updateMatrices();
	m_lights.push_back(l);
	m_sunLight = &m_lights[0];
}

std::vector<World::Mesh>::iterator World::loadObjects(const std::string& fileName, const std::string& materialBaseDir, Resources& resources)
{
	std::string warn;
	std::string errs;
	m_materialBaseDir = materialBaseDir;
	m_shapes.clear();
	m_materials.clear();
	tinyobj::LoadObj(&m_attrib, &m_shapes, &m_materials, &warn, &errs, fileName.c_str(), materialBaseDir.c_str(), true);

	int oldSize = m_objects.size();

	initializeBuffers(resources);
	m_materialBaseDir = "";

	for (int i = 0; i < m_objects.size(); i++)
	{
		if (m_objects[i].name == "sunObject_")
		{
			m_sunObject = &m_objects[i];
			break;
		}
	}

	return m_objects.begin() + oldSize;
}

void World::initializeBuffers(Resources& resources)
{
	auto device = resources.getDevice();

	auto foundIt = m_textures.find("defaultT");
	if (foundIt == m_textures.end())
	{
		m_textures["defaultT"] = resources.loadTexture("default.png", false);
	}

	for (auto& shape : m_shapes)
	{
		auto& mesh = shape.mesh;
		//std::vector<unsigned int> indices;
		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<float> tcoords;
		glm::vec3 minPoint = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		glm::vec3 maxPoint = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);

		//indices.reserve(mesh.indices.size());

		for (size_t f = 0; f < mesh.indices.size() / 3; f++)
		{
				tinyobj::index_t idx0 = mesh.indices[3 * f + 0];
				tinyobj::index_t idx1 = mesh.indices[3 * f + 1];
				tinyobj::index_t idx2 = mesh.indices[3 * f + 2];
				//indices.push_back(idx0.vertex_index);
				//indices.push_back(idx1.vertex_index);
				//indices.push_back(idx2.vertex_index);
			//indices.push_back(ind.vertex_index);

				float v[3][3];
				for (int k = 0; k < 3; k++) {
					int f0 = idx0.vertex_index;
					int f1 = idx1.vertex_index;
					int f2 = idx2.vertex_index;

					v[0][k] = m_attrib.vertices[3 * f0 + k];
					v[1][k] = m_attrib.vertices[3 * f1 + k];
					v[2][k] = m_attrib.vertices[3 * f2 + k];

				}
				glm::vec3 p[3] =
				{
					{v[0][0], v[0][1], v[0][2] },
					{v[1][0], v[1][1], v[1][2] },
					{v[2][0], v[2][1], v[2][2] }
				};
				for (int i = 0; i < 3; ++i)
				{
					minPoint.x = std::min(minPoint.x, p[i].x);
					minPoint.y = std::min(minPoint.y, p[i].y);
					minPoint.z = std::min(minPoint.z, p[i].z);

					maxPoint.x = std::max(maxPoint.x, p[i].x);
					maxPoint.y = std::max(maxPoint.y, p[i].y);
					maxPoint.z = std::max(maxPoint.z, p[i].z);
				}

			vertices.push_back(v[0][0]);
			vertices.push_back(v[0][1]);
			vertices.push_back(v[0][2]);

			vertices.push_back(v[1][0]);
			vertices.push_back(v[1][1]);
			vertices.push_back(v[1][2]);

			vertices.push_back(v[2][0]);
			vertices.push_back(v[2][1]);
			vertices.push_back(v[2][2]);

			float n[3][3];
			int nf0 = idx0.normal_index;
			int nf1 = idx1.normal_index;
			int nf2 = idx2.normal_index;

			for (int k = 0; k < 3; k++) {
				n[0][k] = m_attrib.normals[3 * nf0 + k];
				n[1][k] = m_attrib.normals[3 * nf1 + k];
				n[2][k] = m_attrib.normals[3 * nf2 + k];
			}

			normals.push_back(n[0][0]);
			normals.push_back(n[0][1]);
			normals.push_back(n[0][2]);

			normals.push_back(n[1][0]);
			normals.push_back(n[1][1]);
			normals.push_back(n[1][2]);

			normals.push_back(n[2][0]);
			normals.push_back(n[2][1]);
			normals.push_back(n[2][2]);

			float tc[3][2];
			tc[0][0] = m_attrib.texcoords[2 * idx0.texcoord_index];
			tc[0][1] = 1.0f - m_attrib.texcoords[2 * idx0.texcoord_index + 1];
			tc[1][0] = m_attrib.texcoords[2 * idx1.texcoord_index];
			tc[1][1] = 1.0f - m_attrib.texcoords[2 * idx1.texcoord_index + 1];
			tc[2][0] = m_attrib.texcoords[2 * idx2.texcoord_index];
			tc[2][1] = 1.0f - m_attrib.texcoords[2 * idx2.texcoord_index + 1];

			tcoords.push_back(tc[0][0]);
			tcoords.push_back(tc[0][1]);

			tcoords.push_back(tc[1][0]);
			tcoords.push_back(tc[1][1]);

			tcoords.push_back(tc[2][0]);
			tcoords.push_back(tc[2][1]);
		}
		m_objects.push_back({});

		auto& material = m_materials[mesh.material_ids[0]];
		material.diffuse_texname;
		auto foundIt = m_textures.find(material.diffuse_texname.empty() ? "defaultT": material.diffuse_texname);
		ID3D11ShaderResourceView* diffuse = nullptr;
		if (foundIt == m_textures.end())
		{
			std::string path = m_materialBaseDir + material.diffuse_texname;
			Texture t = resources.loadTexture(path.c_str());
			diffuse = t.m_SRV.Get();
			m_textures[material.diffuse_texname] = t;
		}
		else
		{
			diffuse = foundIt->second.m_SRV.Get();
		}

		auto& ob = m_objects.back();
		ob.maxCoord = maxPoint;
		ob.minCoord = minPoint;
		ob.name = shape.name;
		ob.albedo = diffuse;
		ob.numIndices = mesh.indices.size();//indices.size();
		/*D3D11_BUFFER_DESC buffDesc;
		buffDesc.ByteWidth = sizeof(unsigned int) * indices.size();
		buffDesc.Usage = D3D11_USAGE_DEFAULT;
		buffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffDesc.CPUAccessFlags = 0;
		buffDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA subdata;
		subdata.pSysMem = indices.data();
		device->CreateBuffer(&buffDesc, &subdata, &ob.indexBuffer);
		*/
		/////
		{
			D3D11_BUFFER_DESC buffDesc;
			buffDesc.ByteWidth = sizeof(float) * vertices.size();
			buffDesc.Usage = D3D11_USAGE_DEFAULT;
			buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffDesc.CPUAccessFlags = 0;
			buffDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA subdata;
			subdata.pSysMem = vertices.data();
			device->CreateBuffer(&buffDesc, &subdata, &ob.vert_vb);
		}

		{
			D3D11_BUFFER_DESC buffDesc;
			buffDesc.ByteWidth = sizeof(float) * normals.size();
			buffDesc.Usage = D3D11_USAGE_DEFAULT;
			buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffDesc.CPUAccessFlags = 0;
			buffDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA subdata;
			subdata.pSysMem = normals.data();
			device->CreateBuffer(&buffDesc, &subdata, &ob.norm_vb);
		}

		{
			D3D11_BUFFER_DESC buffDesc;
			buffDesc.ByteWidth = sizeof(float) * tcoords.size();
			buffDesc.Usage = D3D11_USAGE_DEFAULT;
			buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffDesc.CPUAccessFlags = 0;
			buffDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA subdata;
			subdata.pSysMem = tcoords.data();
			device->CreateBuffer(&buffDesc, &subdata, &ob.tcoords_vb);
		}
		/////
	}
}

void World::deinitializeBuffers()
{
	for (auto& mesh : m_objects)
	{
		if (mesh.indexBuffer) mesh.indexBuffer->Release();
		if (mesh.norm_vb) mesh.norm_vb->Release();
		if (mesh.tcoords_vb) mesh.tcoords_vb->Release();
		if (mesh.vert_vb) mesh.vert_vb->Release();
	}
	m_textures.clear();
}

void World::addLight(Light l)
{
	m_lights.push_back(l);
	for (int i = 0; i < m_lights.size(); i++)
	{
		if (m_lights[i].m_type == Light::Type::Directional)
		{
			m_sunLight = &m_lights[i];
			break;
		}
	}
}

void World::updateSun(float dt)
{
	const float rotSpeed = 360.0f / 60.0 * dt;

	m_sunAngle += rotSpeed;
	if (m_sunAngle > 360.0f)
	{
		m_sunAngle = 0.0f;
	}

	m_sunLight->m_position.x = 500.0f * cos(glm::radians(m_sunAngle));
	m_sunLight->m_position.y = 500.0f * sin(glm::radians(m_sunAngle));

	m_sunObject->worldMatrix = glm::translate(m_sunLight->m_position) * glm::scale(glm::vec3{ 5.0f,5.0f,5.0f });

	m_sunLight->m_direction = -m_sunLight->m_position;
	m_sunLight->updateMatrices();

	m_isDay = (m_sunAngle >= 0.0f) && (m_sunAngle < 180.0f);
	m_sunLight->enabled = m_isDay;

	m_sunLight->m_intensity = m_sunColorDay;
	float Int = 85.0f;

	if (m_sunAngle >= 0.0f && m_sunAngle < 60.0f)
	{
		m_sunLight->m_intensity = glm::lerp(m_sunColorMorning, m_sunColorDay, m_sunAngle / 60.0f);
		Int = glm::lerp(2.0f, 5.0f, m_sunAngle / 60.0f);
	}
	if (m_sunAngle >= 120.0f && m_sunAngle < 180.0)
	{
		m_sunLight->m_intensity = glm::lerp(m_sunColorDay, m_sunColorMorning, (m_sunAngle - 120.0f) / 60.0f);
		Int = glm::lerp(5.0f, 2.0f, (m_sunAngle - 120.0f) / 60.0f);
	}

	m_sunLight->m_intensity *= Int;

}
