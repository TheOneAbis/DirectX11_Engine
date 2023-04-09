#pragma once

#include <memory>
#include "MagicMirror.h"

class MagicMirrorManager : public GameEntity
{
public:

	std::shared_ptr<Camera> playerCam; // pointer to player's camera
	std::shared_ptr<Camera> mirrorCams[2];   // pointers to mirror cameras
	MagicMirror mirrors[2]; // mirrors

	MagicMirrorManager(std::shared_ptr<Camera> playerCam, 
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	void Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) override;
	
	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camPtr) override;

	MagicMirror* GetMirror(int index);

private:

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mirrorTextures;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mirrorTarget;
};