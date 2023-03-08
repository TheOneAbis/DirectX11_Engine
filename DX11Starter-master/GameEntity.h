#pragma once

#include <memory>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

class GameEntity
{
private:

	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	float textureScale;

public:

	GameEntity();
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> material);
	Transform* GetTransform();
	void SetTextureUniformScale(float scale);

	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camPtr);
};