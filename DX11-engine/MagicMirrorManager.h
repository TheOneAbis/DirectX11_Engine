#pragma once

#include <memory>
#include "MagicMirror.h"

class MagicMirrorManager : public GameEntity
{
public:

	std::shared_ptr<Camera> playerCam; // pointer to player's camera
	Camera mirrorCam; // this mirror's camera
	MagicMirror mirrors[2];

	MagicMirrorManager(std::shared_ptr<Camera> playerCam);

	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) override;

private:

	/*DirectX::XMFLOAT3 mirror2CamPosOffset;
	DirectX::XMFLOAT3 mirror2CamRotOffset;*/

	DirectX::XMFLOAT4X4 playerCamMirrorMat;
	DirectX::XMFLOAT4X4 mirrorCamMirrorMat;

	void UpdateMirrorMatrices();
};