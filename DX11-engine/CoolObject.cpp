#include "CoolObject.h"
#include <iostream>

using namespace std;
using namespace DirectX;

// Constructors call their superclass counterparts
CoolObject::CoolObject() : GameEntity()
{
	totalTime = 0.0f;
	mousePos = { 0.0f, 0.0f };
	body = Rigidbody();
	body.gravity = { 0, -9.8f, 0 };
	this->AddComponent<Rigidbody>(body);
};
CoolObject::CoolObject(shared_ptr<Mesh> mesh, shared_ptr<Material> material)
	: GameEntity(mesh, material) 
{
	totalTime = 0.0f;
	mousePos = { 0.0f, 0.0f };
	body.gravity = { 0, -9.8f, 0 };
	this->AddComponent<Rigidbody>(body);
};

void CoolObject::Init()
{
	GetTransform()->SetPosition(-4, 0, 0);
}

void CoolObject::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	totalTime += deltaTime;
	mousePos = 
	{ 
		(float)Input::GetInstance().GetMouseX(), 
		(float)Input::GetInstance().GetMouseY() 
	};
}

void CoolObject::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camPtr)
{
	// Do any routine prep work for the material's shaders (i.e. loading stuff)
	GetMaterial()->PrepareMaterial();

	std::shared_ptr<SimpleVertexShader> vs = GetMaterial()->GetVS();
	std::shared_ptr<SimplePixelShader> ps = GetMaterial()->GetPS();

	// Strings here MUST  match variable names in your shader’s cbuffer!
	vs->SetMatrix4x4("world", GetTransform()->GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTranspose", GetTransform()->GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camPtr->GetView());
	vs->SetMatrix4x4("projection", camPtr->GetProjection());

	vs->CopyAllBufferData(); // Adjust “vs” variable name if necessary

	ps->SetFloat2("mousePos", mousePos);
	ps->SetFloat("time", totalTime);

	ps->CopyAllBufferData();

	// Set the shaders for this entity
	GetMaterial()->GetVS()->SetShader();
	GetMaterial()->GetPS()->SetShader();

	// Draw the mesh
	GetMesh()->Draw();

	// reset the SRV's and samplers for the next time so shader is fresh for a different material
	GetMaterial()->ResetTextureData();
}