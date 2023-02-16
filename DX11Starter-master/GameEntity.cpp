#include "GameEntity.h"
#include "BufferStructs.h"

using namespace std;
using namespace DirectX;

GameEntity::GameEntity(shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
}

shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}

void GameEntity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
	shared_ptr<Camera> camPtr)
{
	VertexShaderExternalData vsData = {};
	vsData.worldMatrix = transform.GetWorldMatrix();
	vsData.viewMatrix = camPtr->GetView();
	vsData.projMatrix = camPtr->GetProjection();

	// Holds a pointer to the resource's memory after mapping occurs
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	// copy data from vsData into mapedBuffer
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	context->Unmap(vsConstantBuffer.Get(), 0);

	// Bind the constant buffer to the cbuffer register in GPU memory
	context->VSSetConstantBuffers(
		0,   // Which slot (register) to bind the buffer to?
		1,   // How many are we activating? Can do multiple at once
		vsConstantBuffer.GetAddressOf()); // Array of buffers (or the address of one)

	// Draw the mesh
	mesh->Draw();
}