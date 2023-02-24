#include "GameEntity.h"

using namespace std;
using namespace DirectX;

GameEntity::GameEntity()
{
	this->mesh = nullptr;
	this->material = nullptr;
}

GameEntity::GameEntity(shared_ptr<Mesh> mesh, shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material;
}

shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}

void GameEntity::SetMaterial(shared_ptr<Material> material)
{
	this->material = material;
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}

void GameEntity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	shared_ptr<Camera> camPtr)
{
	// Set the shaders for this entity
	material->GetVS()->SetShader();
	material->GetPS()->SetShader();

	std::shared_ptr<SimpleVertexShader> vs = material->GetVS();
	vs->SetMatrix4x4("world", transform.GetWorldMatrix()); // Strings here MUST 
	vs->SetMatrix4x4("view", camPtr->GetView()); // match variable names in your
	vs->SetMatrix4x4("projection", camPtr->GetProjection()); // shader’s cbuffer!

	vs->CopyAllBufferData(); // Adjust “vs” variable name if necessary

	std::shared_ptr<SimplePixelShader> ps = material->GetPS();
	ps->SetFloat4("colorTint", material->GetColor()); 
	ps->SetFloat("specularAmt", material->GetSpecular());

	ps->CopyAllBufferData();
	//VertexShaderExternalData vsData = {};
	//vsData.worldMatrix = transform.GetWorldMatrix();
	//vsData.viewMatrix = camPtr->GetView();
	//vsData.projMatrix = camPtr->GetProjection();
	//vsData.colorTint = material->GetColor();

	//// Holds a pointer to the resource's memory after mapping occurs
	//D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	//context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	//// copy data from vsData into mappedBuffer
	//memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	//context->Unmap(vsConstantBuffer.Get(), 0);

	//// Bind the constant buffer to the cbuffer register in GPU memory
	//context->VSSetConstantBuffers(
	//	0,   // Which slot (register) to bind the buffer to?
	//	1,   // How many are we activating? Can do multiple at once
	//	vsConstantBuffer.GetAddressOf()); // Array of buffers (or the address of one)

	// Draw the mesh
	mesh->Draw();
}