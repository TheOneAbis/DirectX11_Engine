#include "Skybox.h"
#include "Helpers.h"
#include "WICTextureLoader.h"

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
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<SimpleVertexShader> vertexShader,
	std::shared_ptr<SimplePixelShader> pixelShader,
	const wchar_t* rightTexturePath,
	const wchar_t* leftTexturePath,
	const wchar_t* upTexturePath,
	const wchar_t* downTexturePath,
	const wchar_t* frontTexturePath,
	const wchar_t* backTexturePath)
	:
	skyMesh(mesh),
	sampler(samplerState),
	skyVS(vertexShader),
	skyPS(pixelShader),
	srv(CreateCubemap(rightTexturePath, leftTexturePath, upTexturePath, downTexturePath, frontTexturePath, backTexturePath, device, context))
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
void Skybox::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> cam)
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

void Skybox::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	context->RSSetState(rasterizerState.Get());
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	// Set vertex shader w/ camera view and projection
	skyVS->SetShader();
	skyVS->SetMatrix4x4("view", view);
	skyVS->SetMatrix4x4("projection", projection);
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

// --------------------------------------------------------
// Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face. Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Skybox::CreateCubemap(
	const wchar_t* right,
	const wchar_t* left,
	const wchar_t* up,
	const wchar_t* down,
	const wchar_t* front,
	const wchar_t* back,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
	// - Explicitly NOT generating mipmaps, as we don't need them for the sky!
	// - Order matters here! +X, -X, +Y, -Y, +Z, -Z
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};
	CreateWICTextureFromFile(device.Get(), right, (ID3D11Resource**)textures[0].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), left, (ID3D11Resource**)textures[1].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), up, (ID3D11Resource**)textures[2].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), down, (ID3D11Resource**)textures[3].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), front, (ID3D11Resource**)textures[4].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), back, (ID3D11Resource**)textures[5].GetAddressOf(), 0);
	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first texture
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);
	// Describe the resource for the cube map, which is simply
	// a "texture 2d array" with the TEXTURECUBE flag set.
	// This is a special GPU resource format, NOT just a
	// C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6; // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0; // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width; // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1; // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // A CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;
	// Create the final texture resource to hold the cube map
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	device->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());
	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texture
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0, // Which mip (zero, since there's only one)
			i, // Which array element?
			1); // How many mip levels are in the texture?
		// Copy from one resource (texture) to another
		context->CopySubresourceRegion(
			cubeMapTexture.Get(), // Destination resource
			subresource, // Dest subresource index (one of the array elements)
			0, 0, 0, // XYZ location of copy
			textures[i].Get(), // Source resource
			0, // Source subresource index (we're assuming there's only one)
			0); // Source subresource "box" of data to copy (zero means the whole thing)
	}
	// At this point, all of the faces have been copied into the
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format; // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1; // Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0; // Index of the first mip we want to see
	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());
	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}

std::shared_ptr<SimplePixelShader> Skybox::GetPS()
{
	return skyPS;
}

void Skybox::SetPS(std::shared_ptr<SimplePixelShader> newPS)
{
	skyPS = newPS;
}