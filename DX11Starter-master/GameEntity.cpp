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
// Init() is meant to be overriden bu subclasses
void GameEntity::Init() {}

// Update() is meant to be overriden by subclasses
void GameEntity::Update(float deltaTime) {}

void GameEntity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	shared_ptr<Camera> camPtr)
{
	// Do any routine prep work for the material's shaders (i.e. loading stuff)
	material->PrepareMaterial();

	std::shared_ptr<SimpleVertexShader> vs = material->GetVS();
	std::shared_ptr<SimplePixelShader> ps = material->GetPS();

	// Strings here MUST  match variable names in your shader’s cbuffer!
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camPtr->GetView());
	vs->SetMatrix4x4("projection", camPtr->GetProjection());

	vs->CopyAllBufferData(); // Adjust “vs” variable name if necessary
	
	ps->SetFloat4("colorTint", material->GetColor()); 
	ps->SetFloat("roughness", material->GetRoughness());
	ps->SetFloat3("cameraPosition", camPtr->GetTransform().GetPosition());
	ps->SetFloat("textureScale", textureScale);

	ps->CopyAllBufferData();

	// Set the shaders for this entity
	material->GetVS()->SetShader();
	material->GetPS()->SetShader();

	// Draw the mesh
	mesh->Draw();

	// reset the SRV's and samplers for the next time so shader is fresh for a different material
	material->ResetTextureData();
}