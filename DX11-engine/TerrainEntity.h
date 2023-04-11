#pragma once
#include "GameEntity.h"
#include "Terrain.h"
#include "Helpers.h"

class TerrainEntity : public GameEntity
{
public:

	DirectX::XMFLOAT2 noiseDensity;

	TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material, DirectX::XMFLOAT2 noiseDensity);

	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) override;

private:

	float noiseOffset;
	std::shared_ptr<Terrain> terrainMesh;
	//PerlinObject perlin;

};