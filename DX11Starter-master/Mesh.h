#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "DXCore.h"
#include "Vertex.h"

class Mesh 
{
private:

	// Vertex and index buffers, context pointer
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	// Hold num indices in index buffer
	unsigned int indexCount;

public:

	Mesh(Vertex* vertices, 
		unsigned int numVerts, 
		unsigned int* indices, 
		unsigned int numIndices, 
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	void Draw();

};