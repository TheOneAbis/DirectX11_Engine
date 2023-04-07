#pragma once

#include "GameEntity.h"
#include "Collider.h"
#include "Rigidbody.h"

class CoolObject : public GameEntity
{

public:

	CoolObject();
	CoolObject(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	void Init() override;
	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) override;
	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camPtr) override;

	const Rigidbody& GetRigidbody();

private:

	float totalTime;
	DirectX::XMFLOAT2 mousePos;
	Rigidbody rigidbody;

};