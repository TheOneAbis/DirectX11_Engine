#include "Camera.h"
#include <iostream>
#define PI 3.14159265

using namespace DirectX;

Camera::Camera(float aspectRatio, float fov, float nearClip, float farClip, XMFLOAT3 position, XMFLOAT3 rotation, float movementSpeed, float mouseSens)
{
	transform = Transform();
	transform.SetPosition(position);
	transform.SetRotation(rotation);
	moveSpeed = movementSpeed;
	sensitivity = mouseSens;
	this->fov = fov;
	this->nearClip = nearClip;
	this->farClip = farClip;

	UpdateProjectionMatrix(aspectRatio);
	UpdateViewMatrix();
}

void Camera::Update(float dt)
{
	Input& input = Input::GetInstance();
	if (input.KeyDown('W')) { transform.MoveRelative(0, 0, dt); }        // FORWARD
	if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -dt); }         // BACKWARD
	if (input.KeyDown('A')) { transform.MoveRelative(-dt, 0, 0); }        // LEFT
	if (input.KeyDown('D')) { transform.MoveRelative(dt, 0, 0); }         // RIGHT
	if (input.KeyDown(VK_SPACE)) { transform.MoveAbsolute(0, dt, 0); }    // UP
	if (input.KeyDown(VK_CONTROL)) { transform.MoveAbsolute(0, -dt, 0); } // DOWN

	if (input.MouseLeftDown())
	{
		int cursorMovementX = input.GetMouseXDelta();
		int cursorMovementY = input.GetMouseYDelta();
		
		transform.SetRotation(transform.GetPitchYawRoll().x + ((float)cursorMovementY * sensitivity / 100.0f), 
			transform.GetPitchYawRoll().y + (float)cursorMovementX * sensitivity / 100.0f,
			transform.GetPitchYawRoll().z);

		// Clamp Pitch to +-PI/2 (slightly less than that)
		if (transform.GetPitchYawRoll().x > PI / 2.01f) 
			transform.SetRotation(PI / 2.01f, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
		else if (transform.GetPitchYawRoll().x < -PI / 2.01f) 
			transform.SetRotation(-PI / 2.01f, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
	}

	UpdateViewMatrix();
}

// Updates the camera's projection matrix
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(fov * (PI / 180.0f), aspectRatio, nearClip, farClip));
}

// Update the camera's view matrix
void Camera::UpdateViewMatrix()
{
	XMStoreFloat4x4(&view, XMMatrixLookToLH(
		XMLoadFloat3(&transform.GetPosition()), // position
		XMLoadFloat3(&transform.GetForward()),  // look direction
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));  // global up
}

// Matrix Getters
XMFLOAT4X4 Camera::GetView()
{
	return view;
}

XMFLOAT4X4 Camera::GetProjection()
{
	return projection;
}

Transform& Camera::GetTransform()
{
	return transform;
}