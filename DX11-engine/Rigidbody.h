#pragma once
#include <DirectXMath.h>

struct Rigidbody
{
	DirectX::XMFLOAT3 gravity;
	float friction;
	float linearDamping;
	float angularDamping;

	DirectX::XMFLOAT3 linearVelocity;
	DirectX::XMFLOAT3 angularVelocity;
};