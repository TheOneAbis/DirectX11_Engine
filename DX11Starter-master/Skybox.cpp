#include "Skybox.h"
#include "Helpers.h"

using namespace DirectX;
using namespace std;

// Create a new skybox using an existing vertex & pixel shader, as well as an SRV
Skybox::Skybox(std::shared_ptr<Mesh> mesh,
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	std::shared_ptr<SimpleVertexShader> vertexShader,
	std::shared_ptr<SimplePixelShader> pixelShader,
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) 
	:
	skyMesh(mesh),
	sampler(samplerState),
	skyVS(vertexShader),
	skyPS(pixelShader),
	srv(srv)
{
	// Create rasterizer description and create the rasterizer state from it
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());

	// Create the depth stencil descriptiona and create the depth stencil state from it
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // so it won't fight w/ max depth view distance, since depth buffer starts each frame with a depth of 1
	device->CreateDepthStencilState(&dsDesc, depthStencilState.GetAddressOf());
}

// Create a new skybox using the file paths to vertex and pixel shaders (the compiled .cso files), and paths to textures
Skybox::Skybox(std::shared_ptr<Mesh> mesh,
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	std::shared_ptr<SimpleVertexShader> vertexShader,
	std::shared_ptr<SimplePixelShader> pixelShader,
	wchar_t* rightTexturePath,
	wchar_t* leftTexturePath,
	wchar_t* upTexturePath,
	wchar_t* downTexturePath,
	wchar_t* frontTexturePath,
	wchar_t* backTexturePath)
	:
	skyMesh(mesh),
	sampler(samplerState),
	skyVS(vertexShader),
	skyPS(pixelShader)
{

	// Create rasterizer description and create the rasterizer state from it
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());

	// Create the depth stencil descriptiona and create the depth stencil state from it
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // so it won't fight w/ max depth view distance, since depth buffer starts each frame with a depth of 1
	device->CreateDepthStencilState(&dsDesc, depthStencilState.GetAddressOf());
}


/// <summary>
/// Draws the skybox.
/// </summary>
/// <param name="context">The D3D device context for setting render states</param>
/// <param name="cam">Pointer to the camera to draw the skybox around</param>
void Skybox::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* cam)
{
	context->RSSetState(rasterizerState.Get());
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	// Set vertex shader w/ camera view and projection
	skyVS->SetShader();
	skyVS->SetMatrix4x4("view", cam->GetView());
	skyVS->SetMatrix4x4("projection", cam->GetProjection());
	skyVS->CopyAllBufferData();

	// Set pixel shader w/ sampler state and SRV
	skyPS->SetShader();
	skyPS->SetSamplerState("BasicSampler", sampler);
	skyPS->SetShaderResourceView("SkyboxTexture", srv);
	skyPS->CopyAllBufferData();

	// Draw the skybox, setting vertex & index buffers
	skyMesh->Draw();

	// Reset render states to default
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}