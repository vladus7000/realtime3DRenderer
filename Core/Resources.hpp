#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <map>
#include <vector>
#include <string>

class Resources
{
public:
	enum class TextureResouces
	{
		ShadowMap,
		EnvironmentHDR,
		EnvCubeMapDay,
		EnvCubeMapNight
	};

	Resources();
	~Resources();

	Resources(const Resources& rhs) = delete;
	Resources(Resources&& rhs) = delete;
	Resources& operator=(const Resources& rhs) = delete;
	Resources& operator=(Resources&& rhs) = delete;

	ID3D11Device* getDevice() { return m_device; }
	ID3D11DeviceContext* getContext() { return m_context; }

	Shader createShader(const std::string& fileName, const std::string& vsName, const std::string& psName, std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {});

	void registerTexture(TextureResouces id, Texture* texture);
	void unregisterTexture(TextureResouces id);
	Texture* getTextureResource(TextureResouces id);

	Texture loadTexture(const std::string& fileName, bool createSampler = true);
	Texture loadHDRTexture(const std::string& fileName, bool createSampler = true);
	Texture createDepthStencilTexture(int w, int h);

private:
	IDXGIAdapter* m_adapter = nullptr; // null is primary
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext *m_context = nullptr;

	std::map<TextureResouces, Texture*> m_textureResources;
};