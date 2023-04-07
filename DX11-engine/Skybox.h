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
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader,
		const wchar_t* rightTexturePath, 
		const wchar_t* leftTexturePath,
		const wchar_t* upTexturePath,
		const wchar_t* downTexturePath,
		const wchar_t* frontTexturePath,
		const wchar_t* backTexturePath);

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> cam);

private:

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> skyVS;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

};