#include "Transform.h"

using namespace DirectX;

Transform::Transform()
{
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);

	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
}

void Transform::UpdateMatrices()
{
	XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX r = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(s, r), t);
	XMStoreFloat4x4(&worldMatrix, world);
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, world));
}

// Transformation methods
void Transform::MoveAbsolute(float x, float y, float z)
{
	XMVECTOR posVec = XMVectorAdd(XMLoadFloat3(&position), XMVectorSet(x, y, z, 0.0f));
	XMStoreFloat3(&position, posVec);
}
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	XMVECTOR posVec = XMVectorAdd(XMLoadFloat3(&position), XMLoadFloat3(&offset));
	XMStoreFloat3(&position, posVec);
}
void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR rotVec = XMVectorAdd(XMLoadFloat3(&rotation), XMVectorSet(pitch, yaw, roll, 0.0f));
	XMStoreFloat3(&rotation, rotVec);

}
void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	XMVECTOR rotVec = XMVectorAdd(XMLoadFloat3(&this->rotation), XMLoadFloat3(&rotation));
	XMStoreFloat3(&this->rotation, rotVec);
}
void Transform::Scale(float x, float y, float z)
{
	XMVECTOR scaleVec = XMVectorAdd(XMLoadFloat3(&scale), XMVectorSet(x, y, z, 0.0f));
	XMStoreFloat3(&scale, scaleVec);
}
void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	XMVECTOR scaleVec = XMVectorAdd(XMLoadFloat3(&this->scale), XMLoadFloat3(&scale));
	XMStoreFloat3(&this->scale, scaleVec);
}

// Setters
void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
}
void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
}
void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
}
void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	this->rotation = rotation;
}
void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
}
void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
}

// Getters
DirectX::XMFLOAT3& Transform::GetPosition()
{
	return position;
}
DirectX::XMFLOAT3& Transform::GetPitchYawRoll()
{
	return rotation;
}
DirectX::XMFLOAT3& Transform::GetScale()
{
	return scale;
}
DirectX::XMFLOAT4X4& Transform::GetWorldMatrix()
{
	UpdateMatrices();
	return worldMatrix;
}
DirectX::XMFLOAT4X4& Transform::GetWorldInverseTransposeMatrix()
{
	UpdateMatrices();
	return worldInverseTranspose;
}