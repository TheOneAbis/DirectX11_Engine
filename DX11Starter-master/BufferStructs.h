#pragma once

#include <DirectXMath.h>

struct VertexShaderExternalData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 worldMatrix;
};

struct PixelShaderExternalData
{
	DirectX::XMINT2 mousePos;
};