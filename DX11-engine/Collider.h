#pragma once

#include <memory>
#include "GameEntity.h"

enum ColliderType
{
	BOX_COLLIDER,
	SPHERE_COLLIDER,
	CAPSULE_COLLIDER
};

class Collider
{
public:
	
	std::shared_ptr<GameEntity> gameObject;
	bool isTrigger;
	float radius;
	DirectX::XMFLOAT3 localPosition;
	DirectX::XMFLOAT3 bounds;
	DirectX::XMFLOAT4 localRotation;
	DirectX::XMFLOAT3 localP1;
	DirectX::XMFLOAT3 localP2;

	Collider();
	Collider(std::shared_ptr<GameEntity> gameObject, ColliderType type, bool isTrigger);

	ColliderType GetType();
	bool CollidesWith(Collider* other);

private:

	ColliderType type;
	bool (*collisionFunc)(const Collider* other, const Collider* current);

};