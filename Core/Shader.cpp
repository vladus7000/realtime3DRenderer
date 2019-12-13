#include "Shader.hpp"

Shader::Shader(Microsoft::WRL::ComPtr <ID3D11InputLayout> layout
			, Microsoft::WRL::ComPtr <ID3D11VertexShader> vs
			, Microsoft::WRL::ComPtr <ID3D11PixelShader> ps)
	: m_inputLayout(layout)
	, m_vertexShader(vs)
	, m_pixelShader(ps)
{
}

