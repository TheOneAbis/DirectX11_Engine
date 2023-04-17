#include "GameEntity.h"

using namespace std;
using namespace DirectX;

GameEntity::GameEntity()
{
	this->mesh = nullptr;
	this->material = nullptr;
	textureScale = 1;
	UpdateEnabled = false;
}

GameEntity::GameEntity(shared_ptr<Mesh> mesh, shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material;
	textureScale = 1;
	UpdateEnabled = false;
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

void GameEntity::SetTextureUniformScale(float scale)
{
	textureScale = scale;
}

float GameEntity::GetTextureUniformScale()
{
	return textureScale;
}

// Init() is meant to be overriden bu subclasses
void GameEntity::Init() {}

// Update() is meant to be overriden by subclasses
void GameEntity::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) {}

// Draw the game object using the given camera, setting required shader data
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, shared_ptr<Camera> camPtr)
{
	Draw(context, camPtr->GetTransform().GetPosition(), camPtr->GetView(), camPtr->GetProjection());
}

// Draw the game object using a given camera position, view and projection matrix
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	XMFLOAT3 cameraPos, XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projMatrix)
{
	// Do any routine prep work for the material's shaders (i.e. loading stuff)
	material->PrepareMaterial();

	std::shared_ptr<SimpleVertexShader> vs = material->GetVS();
	std::shared_ptr<SimplePixelShader> ps = material->GetPS();

	// Strings here MUST  match variable names in your shader’s cbuffer!
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", viewMatrix);
	vs->SetMatrix4x4("projection", projMatrix);

	vs->CopyAllBufferData(); // Adjust “vs” variable name if necessary

	ps->SetFloat4("colorTint", material->GetColor());
	ps->SetFloat("roughness", material->GetRoughness());
	ps->SetFloat("metalness", material->GetMetalness());
	ps->SetFloat3("cameraPosition", cameraPos);
	ps->SetFloat("textureScale", textureScale);

	ps->CopyAllBufferData();

	// Set the shaders for this entity
	vs->SetShader();
	ps->SetShader();

	// Draw the mesh
	mesh->Draw();

	// reset the SRV's and samplers for the next time so shader is fresh for a different material
	material->ResetTextureData();
}