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

public:

	bool UpdateEnabled;

	GameEntity();
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> material);
	Transform* GetTransform();
	void SetTextureUniformScale(float scale);

	virtual void Init();
	virtual void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	virtual void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camPtr);

protected:

	float textureScale;
};