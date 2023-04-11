#pragma once

#include <memory>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Component.h"

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

class GameEntity
{
private:

	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	std::unordered_map<std::type_index, Component> components;

public:

	bool UpdateEnabled;

	GameEntity();
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> material);
	Transform* GetTransform();
	void SetTextureUniformScale(float scale);
	float GetTextureUniformScale();

	virtual void Init();
	virtual void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	virtual void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camPtr);

	//// Personal Note: Linker doesn't like separating templated functions bewteen .h and .cpp files. Stupid.

	// Returns this entity's component of the given type
	template <class ComponentType>
	ComponentType& GetComponent() { return components[typeid(ComponentType)]; }

	// Add a component to this entity
	template <class ComponentType>
	bool AddComponent(Component& component)
	{
		// If the map already has a component of this type, don't do anything
		if (components.find(typeid(ComponentType)) != components.end())
			return false;

		// If not, add it to the map
		components[typeid(ComponentType)] = component;
		return true;
	}

protected:

	float textureScale;
};