#pragma once

#include "Shader.hpp"
#include "Texture.hpp"
#include "ConstantBuffer.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <map>
#include <vector>
#include <string>

class Resources
{
public:
	enum class ResoucesID
	{
		//Textures
		ShadowMapC1,
		ShadowMapC2,
		ShadowMapC3,
		EnvironmentHDR,
		EnvCubeMapDay,
		EnvCubeMapNight,
		LitHDRTexture,
		LitHDRTextureBloom,

		//CBs

		//Samplers
		LinearSampler,

		//DepthStates
		EqualDepthState,
		NotEqualDepthState,

		//BlendStates
		DefaultAdditiveBlend
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
	Shader createComputeShader(const std::string& fileName, const std::string& csName);

	template<typename T>
	ConstantBuffer<T> createConstantBuffer();

	void registerResource(ResoucesID id, void* resource);
	void unregisterResource(ResoucesID id);

	template<typename T>
	T* getResource(ResoucesID id)
	{
		T* res = nullptr;
		auto foundIt = m_textureResources.find(id);
		if (foundIt != m_textureResources.end())
		{
			res = static_cast<T*>(foundIt->second);
		}
		return res;
	}

	Texture loadTexture(const std::string& fileName, bool createSampler = true);
	Texture loadHDRTexture(const std::string& fileName, bool createSampler = true);
	Texture createDepthStencilTexture(int w, int h);

private:
	void createDefaultResources();
	void destroyDefaultResources();

	IDXGIAdapter* m_adapter = nullptr; // null is primary
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext *m_context = nullptr;

	std::map<ResoucesID, void*> m_textureResources;
	///Default Resources
	ID3D11SamplerState* m_linearSampler = nullptr;
	ID3D11DepthStencilState* m_equalDepthState = nullptr;
	ID3D11DepthStencilState* m_notEqualDepthState = nullptr;
	ID3D11BlendState* m_defaultAdditiveBlendState = nullptr;
};

template<typename T>
ConstantBuffer<T> Resources::createConstantBuffer()
{
	ConstantBuffer<T> ret;
	D3D11_BUFFER_DESC buffDesc;
	buffDesc.ByteWidth = sizeof(T); // must be multiply of 16
	buffDesc.Usage = D3D11_USAGE_DYNAMIC;
	buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffDesc.MiscFlags = 0;

	ret.flags = D3D11_BIND_CONSTANT_BUFFER;
	ret.size = buffDesc.ByteWidth;
	m_device->CreateBuffer(&buffDesc, nullptr, ret.buffer.GetAddressOf());

	return ret;
}
