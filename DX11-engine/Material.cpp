#include "Material.h"
#include <iostream>

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
	textureBitMask = 0;
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

void Material::SetVS(std::shared_ptr<SimpleVertexShader> newVS)
{
	vs = newVS;
}
void Material::SetPS(std::shared_ptr<SimplePixelShader> newPS)
{
	ps = newPS;
}

void Material::PrepareMaterial()
{
	ps->SetInt("textureBitMask", textureBitMask);
	// Bind the texture SRVs
	for (auto& t : textureSRVs) 
		ps->SetShaderResourceView(t.first.c_str(), t.second);

	// Bind the texture sampler states
	for (auto& s : samplers) 
		ps->SetSamplerState(s.first.c_str(), s.second);
}

// So that if the next material isn't using textures but the same shader, it will not end up using the texture from this material
void Material::ResetTextureData()
{
	ps->SetFloat2("uvOffset", XMFLOAT2(0, 0));
	for (auto& t : textureSRVs)
		ps->SetShaderResourceView(t.first.c_str(), 0);

	for (auto& s : samplers)
		ps->SetSamplerState(s.first.c_str(), 0);
}

void Material::AddTextureSRV(string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	if (strcmp(name.c_str(), "AlbedoMap") == 0) // 1st bit
		textureBitMask |= 1;
	if (strcmp(name.c_str(), "SpecularMap") == 0) // 2nd bit
		textureBitMask |= 2;
	if (strcmp(name.c_str(), "NormalMap") == 0) // 3rd bit
		textureBitMask |= 4;

	textureSRVs.insert({ name, srv });
}

void Material::AddSampler(string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
{
	samplers.insert({ name, samplerState });
}