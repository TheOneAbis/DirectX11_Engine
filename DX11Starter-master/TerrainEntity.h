#pragma once
#include "GameEntity.h"
#include "Terrain.h"

class TerrainEntity : public GameEntity
{
public:

	TerrainEntity();
	TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material);

	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) override;

private:

};