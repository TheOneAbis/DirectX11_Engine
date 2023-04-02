#pragma once
#include "GameEntity.h"
#include "Terrain.h"
#include "Helpers.h"

class TerrainEntity : public GameEntity
{
public:

	DirectX::XMFLOAT2 noiseDensity;

	TerrainEntity();
	TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material, DirectX::XMFLOAT2 noiseDensity);

	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) override;
	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camPtr) override;

private:

	int permutation[512];

	float noiseOffset;
	std::shared_ptr<Terrain> terrainMesh;

};