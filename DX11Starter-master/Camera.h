#pragma once

#include "Input.h"
#include "Transform.h"

enum CamType
{
	Perspective,
	Orthographic
};

class Camera
{
public:

	float fov;
	float nearClip, farClip;
	float moveSpeed;
	float sensitivity;

	Camera(float aspectRatio = 16.0f / 9.0f, 
		float fov = 80.0f,
		float nearClip = 0.1f,
		float farClip = 1000.0f,
		DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0, 0, 0), 
		DirectX::XMFLOAT3 rotation = DirectX::XMFLOAT3(0, 0, 0), 
		float movementSpeed = 2.0f, 
		float mouseSens = 0.5f);

	void Update(float dt);

	Transform& GetTransform();
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();

private:

	Transform transform;
	DirectX::XMFLOAT4X4 view, projection;
	CamType camType;
};