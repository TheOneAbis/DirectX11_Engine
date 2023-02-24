#pragma once

#include "SimpleShader.h"
#include <memory>

class Material
{
public:

	Material(DirectX::XMFLOAT4 color,
		float specular,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader);

	DirectX::XMFLOAT4 GetColor();
	float GetSpecular();
	std::shared_ptr<SimpleVertexShader> GetVS();
	std::shared_ptr<SimplePixelShader> GetPS();

private:

	DirectX::XMFLOAT4 colorTint;
	float specular;
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
};