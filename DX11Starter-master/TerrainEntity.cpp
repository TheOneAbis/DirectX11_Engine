#include "TerrainEntity.h"
#include <iostream>
#include "Helpers.h"

using namespace std;
using namespace DirectX;

TerrainEntity::TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material, XMFLOAT2 noiseDensity) : GameEntity(mesh, material)
{
	this->noiseDensity = noiseDensity;
	this->noiseOffset = 0.0f;

	perlin = PerlinObject(256);

	//// Create array (permutation table) and shuffle it
	//for (int i = 0; i < 256; i++)
	//	permutation[i] = i;

	//// Shuffle the array
	//for (int e = 256 - 1; e > 0; e--) {
	//	const int index = e - 1 == 0 ? 0 : rand() % (e - 1);
	//	const int temp = permutation[e];

	//	permutation[e] = permutation[index];
	//	permutation[index] = temp;
	//}

	//// Duplicate the array
	//for (int i = 256; i < 256 * 2; i++)
	//	permutation[i] = permutation[i - 256];

	// Calculate perlin noise
	for (Vertex& v : mesh->vertices)
		v.Position.y += perlin.FractalBrownianMotion(v.Position.x * noiseDensity.x, v.Position.z * noiseDensity.y, 2);

	// Calculate new normals
	for (int i = 0; i < mesh->vertices.size(); i++)
	{

	}

	mesh->UpdateVBO();
}

void TerrainEntity::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	//noiseOffset += deltaTime * 200.0f;
}

//void TerrainEntity::Draw(
//	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, shared_ptr<Camera> camPtr)
//{
//	// Set permutation table and noise offsets in terrain vertex shader
//	GetMaterial()->GetVS()->SetData("permutation", permutation, sizeof(int) * 512); // not working idk why
//	GetMaterial()->GetVS()->SetFloat("noiseOffset", noiseOffset);
//	GetMaterial()->GetPS()->SetFloat2("uvOffset", XMFLOAT2(noiseOffset, 0));
//	GetMaterial()->GetVS()->SetFloat2("noiseDensity", noiseDensity);
//
//	// Do the rest of the normal drawing stuff
//	GameEntity::Draw(context, camPtr);
//}