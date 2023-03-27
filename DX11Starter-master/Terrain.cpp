#include "Terrain.h"

#include <vector>

using namespace std;
using namespace DirectX;

Terrain::Terrain(unsigned int rows, unsigned int columns, float spaceBetween,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;

	// Create the vertex array
	for (unsigned int i = 0; i < rows; i++) // down
	{
		for (unsigned int j = 0; j < columns; j++) // right
		{
			vertices.push_back({
				XMFLOAT3(i * spaceBetween, 0, j * spaceBetween), // position
				XMFLOAT3(0, 1, 0), // normal
				XMFLOAT2((float)i / (float)rows, (float)j / (float)columns),    // UV
				XMFLOAT3(1, 0, 0)  // tangent
				});
		}
	}

	// Create the index array
	for (unsigned int i = 0; i < columns - 1; i++)
	{
		for (unsigned int j = 0; j < rows - 1; j++)
		{
			// tri 1
			indices.push_back(j + (i * rows));
			indices.push_back(j + (i * rows) + 1);
			indices.push_back(j + (i * rows) + columns); 

			// tri 2
			indices.push_back(j + (i * rows) + 1);
			indices.push_back(j + (i * rows) + columns + 1);
			indices.push_back(j + (i * rows) + columns);
		}
	}

	// set context and index count, then create VBOs and IBOs
	this->context = context;
	this->indexCount = indices.size();
	CreateBuffers(&vertices[0], vertices.size(), &indices[0], device);
}