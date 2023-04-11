#pragma once

#include <memory>
#include "MagicMirror.h"
#include "Lights.h"

class MagicMirrorManager : public GameEntity
{
public:

	MagicMirror mirrors[2]; // mirrors

	MagicMirrorManager(std::shared_ptr<Camera> playerCam, 
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	void Init() override;

	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camPtr);
	
	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camPtr, std::vector<GameEntity*> gameObjects, 
		std::vector<Light> lights, DirectX::XMFLOAT3 ambientColor);

	MagicMirror* GetMirror(int index);

private:

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mirrorTarget;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mirrorSRV;

	DirectX::XMFLOAT4X4 mirrorViews[2];
	DirectX::XMFLOAT4X4 mirrorProjs[2];
	DirectX::XMFLOAT3 mirrorCamPositions[2];
	std::shared_ptr<SimplePixelShader> mirrorViewPS;
};