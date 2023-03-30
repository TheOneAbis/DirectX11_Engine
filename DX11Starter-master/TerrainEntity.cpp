#include "TerrainEntity.h"
#include <iostream>
#include <time.h>

TerrainEntity::TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material) : GameEntity(mesh, material)
{
	srand(time(NULL));
}

void TerrainEntity::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	for (Vertex& v : GetMesh()->vertices)
	{
		v.Position.y = (rand() % 200) / 200.0f - 1;
	}
}