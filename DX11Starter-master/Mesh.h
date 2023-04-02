#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include "DXCore.h"
#include "Vertex.h"

class Mesh 
{
private:

	// Vertex and index buffers, context pointer
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

protected:

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	// Hold num indices in index buffer
	unsigned int indexCount;

	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

	void CreateBuffers(Vertex* vertices,
		unsigned int numVerts,
		unsigned int* indices,
		Microsoft::WRL::ComPtr<ID3D11Device> device);

public:

	std::vector<Vertex> vertices;

	Mesh();

	Mesh(Vertex* vertices, 
		unsigned int numVerts, 
		unsigned int* indices, 
		unsigned int numIndices, 
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	Mesh(const wchar_t* fileName,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	virtual void Draw();

};