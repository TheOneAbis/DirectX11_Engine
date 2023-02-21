#include "Material.h"

using namespace DirectX;
using namespace std;

Material::Material(XMFLOAT4 color, 
	shared_ptr<SimpleVertexShader> vertexShader,
	shared_ptr<SimplePixelShader>  pixelShader)
{
	colorTint = color;
	vs = vertexShader;
	ps = pixelShader;
}

XMFLOAT4 Material::GetColor()
{
	return colorTint;
}

shared_ptr<SimpleVertexShader> Material::GetVS()
{
	return vs;
}

shared_ptr<SimplePixelShader> Material::GetPS()
{
	return ps;
}