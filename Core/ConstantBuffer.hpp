#pragma once

#include <d3d11.h>
#include <wrl.h>

template<typename T>
struct ConstantBuffer
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	unsigned int flags = 0;
	unsigned int size = 0;
};