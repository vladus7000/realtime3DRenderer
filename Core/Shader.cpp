#include "Shader.hpp"

Shader::Shader(ID3D11InputLayout* layout, ID3D11VertexShader* vs, ID3D11PixelShader* ps)
	: m_inputLayout(layout)
	, m_vertexShader(vs)
	, m_pixelShader(ps)
{
	if (m_inputLayout)
	{
		m_inputLayout->AddRef();
	}
	if (m_vertexShader)
	{
		m_vertexShader->AddRef();
	}
	if (m_pixelShader)
	{
		m_pixelShader->AddRef();
	}
}

Shader::Shader(const Shader& rhs)
{
	operator=(rhs);
}

Shader::Shader(Shader&& rhs)
{
	operator=(rhs);
}

void Shader::release()
{
	if (m_inputLayout)
	{
		m_inputLayout->Release();
		m_inputLayout = nullptr;
	}
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}
}

Shader& Shader::operator=(Shader&& rhs)
{
	release();
	if (rhs.m_inputLayout)
	{
		m_inputLayout = rhs.m_inputLayout;
		rhs.m_inputLayout = nullptr;
	}
	if (rhs.m_vertexShader)
	{
		m_vertexShader = rhs.m_vertexShader;
		rhs.m_vertexShader = nullptr;
	}
	if (rhs.m_pixelShader)
	{
		m_pixelShader = rhs.m_pixelShader;
		rhs.m_pixelShader = nullptr;
	}
	return *this;
}

Shader& Shader::operator=(const Shader& rhs)
{
	release();
	if (rhs.m_inputLayout)
	{
		m_inputLayout = rhs.m_inputLayout;
		m_inputLayout->AddRef();
	}
	if (rhs.m_vertexShader)
	{
		m_vertexShader = rhs.m_vertexShader;
		m_vertexShader->AddRef();
	}
	if (rhs.m_pixelShader)
	{
		m_pixelShader = rhs.m_pixelShader;
		m_pixelShader->AddRef();
	}
	return *this;
}

Shader::~Shader()
{
	release();
}
