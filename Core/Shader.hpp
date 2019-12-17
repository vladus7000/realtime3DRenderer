#pragma once

#include <d3d11.h>
#include <wrl.h>

class Shader
{
public:
	Shader() {}
	//graphics pipe
	Shader(Microsoft::WRL::ComPtr <ID3D11InputLayout> layout
		, Microsoft::WRL::ComPtr <ID3D11VertexShader> vs
		, Microsoft::WRL::ComPtr <ID3D11PixelShader> ps);

	//compute pipe
	Shader(Microsoft::WRL::ComPtr<ID3D11ComputeShader> cs);

	ID3D11VertexShader* getVertexShader() const { return m_vertexShader.Get(); }
	ID3D11PixelShader* getPixelShader() const { return m_pixelShader.Get(); }
	ID3D11InputLayout* getInputLayout() const { return m_inputLayout.Get(); }

	ID3D11ComputeShader* getComputeShader() const { return m_computeShader.Get(); }
private:
	//graphics pipe
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	//compute pipe
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
};