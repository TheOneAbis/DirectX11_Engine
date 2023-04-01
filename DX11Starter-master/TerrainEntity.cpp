#include "TerrainEntity.h"
#include <iostream>
#include "Helpers.h"

using namespace std;
using namespace DirectX;

TerrainEntity::TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material, XMFLOAT2 noiseDensity, float height) : GameEntity(mesh, material)
{
	perlin = PerlinObject(256);
	noiseOffset = 0.0f;
	terrainMesh = mesh;
	this->noiseDensity = noiseDensity;
	this->height = height;

	for (unsigned int y = 0; y < mesh->resolution.y; y++)
	{
		for (unsigned int x = 0; x < mesh->resolution.x; x++)
		{
			mesh->vertices[x + mesh->resolution.y * y].Position.y = 
				perlin.Perlin2D(x * 0.01f * noiseDensity.x, y * 0.01f * noiseDensity.y) * height;
		}
	}
}

void TerrainEntity::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	noiseOffset += deltaTime / 2.0f;

	for (unsigned int y = 0; y < terrainMesh->resolution.y; y++)
	{
		for (unsigned int x = 0; x < terrainMesh->resolution.x; x++)
		{
			terrainMesh->vertices[x + terrainMesh->resolution.y * y].Position.y =
				perlin.Perlin2D(x * 0.01f * noiseDensity.x + noiseOffset, y * 0.01f * noiseDensity.y) * height;
		}
	}	
}