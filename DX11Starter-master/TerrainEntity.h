#pragma once
#include "GameEntity.h"
#include "Terrain.h"
#include "Helpers.h"

class TerrainEntity : public GameEntity
{
public:

	DirectX::XMFLOAT2 noiseDensity;
	float height;

	TerrainEntity();
	TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material, DirectX::XMFLOAT2 noiseDensity, float height);

	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) override;

private:

	PerlinObject perlin;
	float noiseOffset;
	std::shared_ptr<Terrain> terrainMesh;

};