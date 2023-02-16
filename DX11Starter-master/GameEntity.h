#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"

class GameEntity
{
private:

	Transform transform;
	std::shared_ptr<Mesh> mesh;

public:

	GameEntity(std::shared_ptr<Mesh> mesh);

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
		std::shared_ptr<Camera> camPtr,
		DirectX::XMFLOAT4 color);
};