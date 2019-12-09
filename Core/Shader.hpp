#pragma once

#include <d3d11.h>

class Shader
{
public:
	Shader() {}
	Shader(ID3D11InputLayout* layout, ID3D11VertexShader* vs, ID3D11PixelShader* ps);
	~Shader();

	Shader(const Shader& rhs);
	Shader(Shader&& rhs);

	Shader& operator=(const Shader& rhs);
	Shader& operator=(Shader&& rhs);

	ID3D11VertexShader* getVertexShader() const { return m_vertexShader; }
	ID3D11PixelShader* getPixelShader() const { return m_pixelShader; }
	ID3D11InputLayout* getInputLayout() const { return m_inputLayout; }

private:
	void release();

private:
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
};