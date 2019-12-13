#pragma once

#include <d3d11.h>
#include <wrl.h>

class Shader
{
public:
	Shader() {}
	Shader(Microsoft::WRL::ComPtr <ID3D11InputLayout> layout
		, Microsoft::WRL::ComPtr <ID3D11VertexShader> vs
		, Microsoft::WRL::ComPtr <ID3D11PixelShader> ps);

	ID3D11VertexShader* getVertexShader() const { return m_vertexShader.Get(); }
	ID3D11PixelShader* getPixelShader() const { return m_pixelShader.Get(); }
	ID3D11InputLayout* getInputLayout() const { return m_inputLayout.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};