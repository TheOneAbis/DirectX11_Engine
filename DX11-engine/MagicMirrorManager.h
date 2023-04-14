#pragma once

#include <memory>
#include "MagicMirror.h"
#include "Lights.h"
#include "Skybox.h"

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
		std::shared_ptr<Camera> camPtr, 
		std::vector<GameEntity*> gameObjects, 
		std::shared_ptr<Skybox> skybox,
		std::vector<Light> lights);

	MagicMirror* GetMirror(int index);

	void ResetMirrorTextures(Camera* cam, Microsoft::WRL::ComPtr<ID3D11Device> device);

private:

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mirrorTarget;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mirrorSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mirrorDSVs[2];

	DirectX::XMFLOAT4X4 mirrorViews[2];
	DirectX::XMFLOAT4X4 mirrorProjs[2];
	DirectX::XMFLOAT3 mirrorCamPositions[2];
	std::shared_ptr<SimplePixelShader> mirrorViewPS;
	std::shared_ptr<SimplePixelShader> skyboxMirrorPS;
};