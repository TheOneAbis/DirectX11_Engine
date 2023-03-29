#pragma once

#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

class Skybox
{
public:

	Skybox(std::shared_ptr<Mesh> mesh, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState, 
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);

	Skybox(std::shared_ptr<Mesh> mesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader,
		wchar_t* rightTexturePath, 
		wchar_t* leftTexturePath,
		wchar_t* upTexturePath,
		wchar_t* downTexturePath,
		wchar_t* frontTexturePath,
		wchar_t* backTexturePath);

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* cam);

private:

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> skyVS;

};