#include "Material.h"

using namespace DirectX;
using namespace std;

Material::Material(XMFLOAT4 color, 
	float roughness,
	shared_ptr<SimpleVertexShader> vertexShader,
	shared_ptr<SimplePixelShader>  pixelShader)
{
	colorTint = color;
	this->roughness = roughness;
	vs = vertexShader;
	ps = pixelShader;
}

XMFLOAT4 Material::GetColor()
{
	return colorTint;
}

float Material::GetRoughness()
{
	return roughness;
}

shared_ptr<SimpleVertexShader> Material::GetVS()
{
	return vs;
}

shared_ptr<SimplePixelShader> Material::GetPS()
{
	return ps;
}