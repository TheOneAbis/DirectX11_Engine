#include "Collider.h"
#include <vector>

using namespace DirectX;
using namespace std;


// Create a default box collider (if using default, you'll need to set its gameObject reference manually)
Collider::Collider()
{
	this->type = BOX_COLLIDER;
	this->isTrigger = false;
	this->gameObject = 0; // to be set externally
	this->radius = 1;
	this->bounds = XMFLOAT3(1, 1, 1);
	this->localP1 = XMFLOAT3(0, -1, 0);
	this->localP2 = XMFLOAT3(0, 1, 0);
	this->localRotation = XMFLOAT4(0, 0, 0, 1);
}

// Create a collider with the given type and if it is a trigger collider or not
Collider::Collider(shared_ptr<GameEntity> gameObject, ColliderType type, bool isTrigger)
{
	this->type = type;
	this->isTrigger = isTrigger;
	this->gameObject = gameObject;
	this->radius = 1;
	this->bounds = XMFLOAT3(1, 1, 1);
	this->localP1 = XMFLOAT3(0, -1, 0);
	this->localP2 = XMFLOAT3(0, 1, 0);
	this->localRotation = XMFLOAT4(0, 0, 0, 1);

	switch (type)
	{
		case BOX_COLLIDER:
			// Set the collision test function to run
			this->collisionFunc = [](const Collider* other, const Collider* current = this)
			{
				return false;
			};
			break;

		case SPHERE_COLLIDER:
			// Set the collision test function to run
			this->collisionFunc = [](const Collider* other, const Collider* current = this)
			{
				float dist2 = -1;
				XMStoreFloat(&dist2, XMVector3LengthSq(XMVectorSubtract(
					XMLoadFloat3(&other->gameObject->GetTransform()->GetPosition()),
					XMLoadFloat3(&current->gameObject->GetTransform()->GetPosition())
				)));
				float minDist = current->radius + other->radius;
				return dist2 <= minDist * minDist;
			};
			break;

		case CAPSULE_COLLIDER:
			// Set the collision test function to run
			this->collisionFunc = [](const Collider* other, const Collider* current = this)
			{
				return false;
			};
			break;
	}
};

// Returns the type of this collider
ColliderType Collider::GetType()
{
	return this->type;
};

bool Collider::CollidesWith(Collider* other)
{
	return collisionFunc(other, this);
}