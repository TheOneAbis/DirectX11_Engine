#pragma once
#include <DirectXMath.h>
#include "Component.h"

class Rigidbody : public Component
{
public:
	DirectX::XMFLOAT3 gravity;
	float friction;
	float linearDamping;
	float angularDamping;

	DirectX::XMFLOAT3 linearVelocity;
	DirectX::XMFLOAT3 angularVelocity;
};