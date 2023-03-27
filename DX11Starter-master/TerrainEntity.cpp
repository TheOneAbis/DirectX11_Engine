#include "TerrainEntity.h"
#include <iostream>

TerrainEntity::TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material) : GameEntity(mesh, material)
{
}

void TerrainEntity::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	//context->Map()
	//GetMesh()->GetVertexBuffer();
}