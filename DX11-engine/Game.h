#pragma once

#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <vector>
#include <memory>
#include "DXCore.h"
#include "SimpleShader.h"
#include "Lights.h"
#include "Skybox.h"

#include "GameEntitySubclassIncludes.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateGeometry();
	void UpdateUI(float deltaTime);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	std::vector<GameEntity*> gameObjects; // pointers for polymorphic behavior
	std::shared_ptr<MagicMirrorManager> mirrorManager;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::shared_ptr<Skybox> skybox;
	std::vector<std::shared_ptr<Camera>> cams;
	std::shared_ptr<Camera> activeCam;
	int camIndex;

	// lights and shadowmap stuff
	std::vector<Light> lights;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRS;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSS;
	std::shared_ptr<SimpleVertexShader> shadowVS;
	DirectX::XMFLOAT3 ambientLight;

	DirectX::XMFLOAT4X4 lightView;
	DirectX::XMFLOAT4X4 lightProj;
	unsigned int shadowMapRes;
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader, customPS;
	std::shared_ptr<SimpleVertexShader> vertexShader;

	// Skybox shaders
	std::shared_ptr<SimpleVertexShader> skyVS;
	std::shared_ptr<SimplePixelShader> skyPS;
};

