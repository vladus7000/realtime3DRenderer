#include "World.hpp"
#include "Renderer.hpp"

#include <D3DX11tex.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

std::vector<World::Mesh>::iterator World::loadObjects(const std::string& fileName, const std::string& materialBaseDir, Renderer& renderer)
{
	std::string warn;
	std::string errs;
	m_materialBaseDir = materialBaseDir;
	m_shapes.clear();
	m_materials.clear();
	tinyobj::LoadObj(&m_attrib, &m_shapes, &m_materials, &warn, &errs, fileName.c_str(), materialBaseDir.c_str(), true);

	int oldSize = m_objects.size();

	initializeBuffers(renderer);
	m_materialBaseDir = "";

	return m_objects.begin() + oldSize;
}

void World::initializeBuffers(Renderer& renderer)
{
	auto device = renderer.getDevice();

	for (auto& shape : m_shapes)
	{
		auto& mesh = shape.mesh;
		//std::vector<unsigned int> indices;
		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<float> tcoords;

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
		auto foundIt = m_textures.find(material.diffuse_texname);
		ID3D11ShaderResourceView* diffuse = nullptr;
		if (foundIt == m_textures.end())
		{
			std::string path = m_materialBaseDir + std::string("/") + material.diffuse_texname;
			D3DX11CreateShaderResourceViewFromFile(device, path.c_str(), 0, 0, &diffuse, 0);

			m_textures[material.diffuse_texname] = diffuse;
		}
		else
		{
			diffuse = foundIt->second;
		}

		auto& ob = m_objects.back();
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

	for (auto& val : m_textures)
	{
		val.second->Release();
	}
}
