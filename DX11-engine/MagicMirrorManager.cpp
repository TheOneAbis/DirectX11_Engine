#include "MagicMirrorManager.h"

using namespace std;
using namespace DirectX;

MagicMirrorManager::MagicMirrorManager(shared_ptr<Camera> playerCam) : GameEntity()
{
	this->playerCam = playerCam;
	this->mirrorCam = Camera(*playerCam.get());
}

void MagicMirrorManager::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Set mirror 2 cam in mirror 2 space, which is player cam in mirror 1 space but negated
	XMVECTOR mirrorPosVec = XMLoadFloat3(&mirrors[0].GetTransform()->GetPosition());
	XMVECTOR mirrorCamPosOffset = XMVectorSubtract(mirrorPosVec,
		XMLoadFloat3(&playerCam->GetTransform().GetPosition())); // cam -> mirror
	
	// Get the axis of rotation between mirror forward and cam's NEGATED forward
	XMVECTOR mirrorForward = XMLoadFloat3(&mirrors[0].GetTransform()->GetForward());
	XMVECTOR axis = XMVector3Cross(mirrorForward,
		XMVectorScale(XMLoadFloat3(&playerCam->GetTransform().GetForward()), -1.0f)
	);

	// Get the angle from the axis length
	float angleRad;
	XMStoreFloat(&angleRad, XMVector3Length(axis));
	angleRad = asinf(angleRad);

	// Calculate result of mirror forward rotated angleRad radians around axis
	// Then, use XMMatrixLookToLH() to create mirror cam's new view matrix
	XMVECTOR rotQuat = XMQuaternionRotationAxis(axis, angleRad);
	XMMATRIX view = XMMatrixLookToLH(
		XMVectorAdd(mirrorPosVec, mirrorCamPosOffset),      // mirror camera position in world space
		XMVector3Rotate(mirrorForward, rotQuat),            // mirror Forward rotated by angle-axis quaternion
		XMLoadFloat3(&mirrors[0].GetTransform()->GetUp())); // mirror Up axis
}

void MagicMirrorManager::UpdateMirrorMatrices()
{

}