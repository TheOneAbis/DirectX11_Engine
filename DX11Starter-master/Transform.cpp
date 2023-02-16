#include "Transform.h"

using namespace DirectX;

Transform::Transform()
{
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);

	forward = XMFLOAT3(0, 0, 1);
	up = XMFLOAT3(0, 1, 0);
	right = XMFLOAT3(1, 0, 0);

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

void Transform::MoveRelative(float x, float y, float z)
{
	// Create the movement vector rotated by transform's current rotation Quat
	XMVECTOR rotVec = XMVector3Rotate(XMVectorSet(x, y, z, 0.0f), 
		XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z));

	// Add the rotated movement vector to transform's current position
	XMStoreFloat3(&position, 
		XMVectorAdd(XMLoadFloat3(&position), rotVec));
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

	// Create the rotation quat
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	// Rotate local right
	XMStoreFloat3(&right, XMVector3Rotate(XMLoadFloat3(&right),
		rotQuat));

	// Rotate local up
	XMStoreFloat3(&up, XMVector3Rotate(XMLoadFloat3(&up),
		rotQuat));

	// Rotate local forward
	XMStoreFloat3(&forward, XMVector3Rotate(XMLoadFloat3(&forward),
		rotQuat));
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
	SetPosition(XMFLOAT3(x, y, z));
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	SetRotation(XMFLOAT3(pitch, yaw, roll));
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	this->rotation = rotation;

	XMFLOAT3 worldRight = XMFLOAT3(1, 0, 0);
	XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);
	XMFLOAT3 worldForward = XMFLOAT3(0, 0, 1);
	// Create the rotation quat
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	// Rotate local right
	XMStoreFloat3(&right, XMVector3Rotate(XMLoadFloat3(&worldRight),
		rotQuat));

	// Rotate local up
	XMStoreFloat3(&up, XMVector3Rotate(XMLoadFloat3(&worldUp),
		rotQuat));

	// Rotate local forward
	XMStoreFloat3(&forward, XMVector3Rotate(XMLoadFloat3(&worldForward),
		rotQuat));
}

void Transform::SetScale(float x, float y, float z)
{
	SetScale(XMFLOAT3(x, y, z));
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
}

// Get Position, Rotation, and Scale
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

// Get Local Right, Up, and Forward
DirectX::XMFLOAT3& Transform::GetRight()
{
	return right;
}

DirectX::XMFLOAT3& Transform::GetUp()
{
	return up;
}

DirectX::XMFLOAT3& Transform::GetForward()
{
	return forward;
}

// Get World and Inverse Transpose Matrices
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