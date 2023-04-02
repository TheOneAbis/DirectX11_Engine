#include "Terrain.h"
#include <vector>

using namespace std;
using namespace DirectX;

Terrain::Terrain(unsigned int rows, unsigned int columns,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	resolution = XMINT2(columns, rows);
	vector<Vertex> vertices;
	vector<unsigned int> indices;

	// Create the vertex array
	for (unsigned int i = 0; i < rows; i++) // up
	{
		for (unsigned int j = 0; j < columns; j++) // right
		{
			vertices.push_back({
				XMFLOAT3(i, 0, j), // position
				XMFLOAT3(0, 1, 0), // normal
				XMFLOAT2(i, j),    // UV
				XMFLOAT3(1, 0, 0)  // tangent
				});
		}
	}

	// Create the index array
	for (unsigned int i = 0; i < rows - 1; i++)
	{
		for (unsigned int j = 0; j < columns - 1; j++)
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
	this->vertices = vertices;
	this->indexCount = (unsigned int)indices.size();
	CreateBuffers(&vertices[0], (unsigned int)vertices.size(), &indices[0], device);
}

//void Terrain::Draw()
//{
//	D3D11_MAPPED_SUBRESOURCE vData = {}; // will hold a POINTER to vertex buffer in GPU memory
//	context->Map(GetVertexBuffer().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vData); // lock vertex buffer and get us pointer to it
//	memcpy(vData.pData, &vertices[0], sizeof(Vertex) * vertices.size()); // copy from vertices to GPU
//	context->Unmap(GetVertexBuffer().Get(), 0); // Unlock vertex buffer w/ new data
//
//	Mesh::Draw(); // call the parent's draw method to draw the terrain on the screen
//}