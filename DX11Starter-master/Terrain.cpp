#include "Terrain.h"

#include <vector>

using namespace std;
using namespace DirectX;

Terrain::Terrain(unsigned int rows, unsigned int columns, float spaceBetween,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	Vertex* vertices = new Vertex[rows * columns];
	int index = 0;
	vector<int> indices;

	// Create the vertices
	for (unsigned int i = 0; i < rows; i++) // down
	{
		for (unsigned int j = 0; j < columns; j++) // right
		{
			vertices[index] = {
				XMFLOAT3(i * spaceBetween, 0, j * spaceBetween), // position
				XMFLOAT3(0, 1, 0), // normal
				XMFLOAT2(0, 0),    // UV
				XMFLOAT3(1, 0, 0)  // tangent
				};
			index++;
		}
	}

	index = 0;
	for (unsigned int i = 0; i < columns - 1; i++)
	{
		for (unsigned int j = 0; j < rows - 1; j++)
		{
			// tri 1
			indices.push_back(j + (i * rows));
			indices.push_back(j + (i * rows) + columns); 
			indices.push_back(j + (i * rows) + 1);

			// tri 2
			indices.push_back(j + (i * rows) + 1);
			indices.push_back(j + (i * rows) + columns);
			indices.push_back(j + (i * rows) + columns + 1);

			index += 6;
		}
	}
	unsigned int* indexArr = new unsigned int[indices.size()];
	for (int i = 0; i < indices.size(); i++)
		indexArr[i] = indices[i];

	Mesh(vertices, rows * columns, indexArr, (unsigned int)indices.size(), device, context);
}