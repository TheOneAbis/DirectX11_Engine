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
		std::shared_ptr<Camera> camPtr, std::vector<GameEntity*> gameObjects, 
		std::shared_ptr<Skybox> skybox, std::vector<Light> lights, DirectX::XMFLOAT3 ambient);

	MagicMirror* GetMirror(int index);

	void ResetMirrors(Camera* cam, Microsoft::WRL::ComPtr<ID3D11Device> device);

private:

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mirrorTargets[2];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mirrorSRVs[2];

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mirrorDSV;

	DirectX::XMFLOAT4X4 mirrorProj;
	DirectX::XMFLOAT4X4 mirrorCamView;
	DirectX::XMFLOAT3 mirrorCamPositions[2];
	DirectX::XMFLOAT3 mirrorCamForwards[2];
	DirectX::XMFLOAT3 mirrorCamUps[2];
	DirectX::XMFLOAT4 mirrorQuatDiff;

	std::shared_ptr<SimplePixelShader> mirrorPS;
	std::shared_ptr<SimplePixelShader> mirrorPSCulled;
	std::shared_ptr<SimplePixelShader> mirrorViewPS;
	std::shared_ptr<SimplePixelShader> skyboxMirrorPS;

	void RenderThroughMirror(int mirrorIndex, int depthIndex, DirectX::XMFLOAT3 mirrorCamPos, DirectX::XMFLOAT3 prevMirrorCamPos,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> viewportTarget,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> viewportDSV,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		DirectX::XMFLOAT2 viewDimensions,
		std::vector<GameEntity*> gameObjects,
		std::shared_ptr<Skybox> skybox,
		std::vector<Light> lights, DirectX::XMFLOAT3 ambient);
};