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

void Material::PrepareMaterial()
{
	ps->SetInt("usesTextures", usesTextures);
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
	// No SRVs - not using textures
	if (textureSRVs.size() == 0)
	{
		usesTextures = false;
		return;
	}

	for (auto& t : textureSRVs)
		ps->SetShaderResourceView(t.first.c_str(), 0);

	for (auto& s : samplers)
		ps->SetSamplerState(s.first.c_str(), 0);
}

void Material::AddTextureSRV(string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name, srv });
}

void Material::AddSampler(string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
{
	samplers.insert({ name, samplerState });
}