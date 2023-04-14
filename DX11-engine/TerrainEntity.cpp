#include "TerrainEntity.h"
#include <iostream>
#include "Helpers.h"

using namespace std;
using namespace DirectX;

TerrainEntity::TerrainEntity(std::shared_ptr<Terrain> mesh, std::shared_ptr<Material> material, XMFLOAT2 noiseDensity) : GameEntity(mesh, material)
{
	this->noiseDensity = noiseDensity;
	this->noiseOffset = 0.0f;

	int permutation[256] = 
	{ 
		151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7,
		225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247,
		120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
		88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134,
		139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220,
		105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80,
		73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86,
		164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38,
		147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189,
		28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101,
		155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232,
		178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12,
		191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181,
		199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236,
		205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 
	};
	PerlinObject perlin = PerlinObject(permutation, 256);

	// Calculate perlin noise
	for (Vertex& v : mesh->vertices)
		v.Position.y += perlin.FractalBrownianMotion(v.Position.x * noiseDensity.x, v.Position.z * noiseDensity.y, 2);
	unsigned int s = mesh->vertices.size();

	// Calculate new normals
	for (unsigned int i = 0; i < mesh->resolution.y; i++)
	{
		for (unsigned int j = 0; j < mesh->resolution.x; j++)
		{
			Vertex main = mesh->vertices[j + (i * mesh->resolution.x)];
			int iLeft = j + (i * mesh->resolution.x) - 1;
			int iUp = j + ((i - 1) * mesh->resolution.x);
			Vertex* right = j + (i * mesh->resolution.x) + 1 < s ? &mesh->vertices[j + (i * mesh->resolution.x) + 1] : 0;
			Vertex* down = j + ((i + 1) * mesh->resolution.x) < s ? &mesh->vertices[j + ((i + 1) * mesh->resolution.x)] : 0;
			Vertex* left = iLeft >= 0 ? &mesh->vertices[iLeft] : 0;
			Vertex* up = iUp >= 0 ? &mesh->vertices[iUp] : 0;

			XMVECTOR vMain = XMLoadFloat3(&main.Position);
			XMVECTOR vToRight = right == 0 ? XMVectorSet(0, 0, 0, 0) : XMVectorSubtract(XMLoadFloat3(&right->Position), vMain);
			XMVECTOR vToDown = down == 0 ? XMVectorSet(0, 0, 0, 0) : XMVectorSubtract(XMLoadFloat3(&down->Position), vMain);
			XMVECTOR vToLeft = left == 0 ? XMVectorSet(0, 0, 0, 0) : XMVectorSubtract(XMLoadFloat3(&left->Position), vMain);
			XMVECTOR vToUp = up == 0 ? XMVectorSet(0, 0, 0, 0) : XMVectorSubtract(XMLoadFloat3(&up->Position), vMain);

			XMVECTOR crossRightDown = XMVector3Cross(vToRight, vToDown);
			XMVECTOR crossDownLeft = XMVector3Cross(vToDown, vToLeft);
			XMVECTOR crossLeftUp = XMVector3Cross(vToLeft, vToUp);
			XMVECTOR crossUpRight = XMVector3Cross(vToUp, vToRight);
			
			XMStoreFloat3(&mesh->vertices[j + (i * mesh->resolution.x)].Normal,
				XMVector3Normalize(XMVectorAdd(
					XMVectorAdd(crossRightDown, crossDownLeft), 
					XMVectorAdd(crossLeftUp, crossUpRight))));
		}
	}

	mesh->UpdateVBO();
}

void TerrainEntity::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	//noiseOffset += deltaTime * 200.0f;
}