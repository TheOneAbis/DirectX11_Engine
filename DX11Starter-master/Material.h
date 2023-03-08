#pragma once

#include "SimpleShader.h"
#include <memory>
#include <unordered_map>

class Material
{
public:

	Material(DirectX::XMFLOAT4 color,
		float roughness,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader);

	DirectX::XMFLOAT4 GetColor();
	float GetRoughness();
	std::shared_ptr<SimpleVertexShader> GetVS();
	std::shared_ptr<SimplePixelShader> GetPS();

	void PrepareMaterial();
	void ResetTextureData();

	// To add textures to a material
	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState);

private:

	DirectX::XMFLOAT4 colorTint;
	float roughness;
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;

	// texture SRV and sampler maps
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

	bool usesTextures;
};