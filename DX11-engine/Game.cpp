#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "WICTextureLoader.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include <iostream>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	// Initialize random seed for future use as well
	srand((unsigned int)time(NULL));
	camIndex = 0;
	lightView = {};
	lightProj = {};
	shadowMapRes = 0;
	ambientLight = {};
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Clean up the game entities (as these were made on the heap)
	for (GameEntity* obj : gameObjects)
		delete obj;

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	// Init the cameras
	cams.push_back(std::make_shared<Camera>(Perspective, (float)windowWidth, (float)windowHeight, 80.0f, 0.1f, 1000.0f, XMFLOAT3(0, 0, -3.0f)));
	cams.push_back(std::make_shared<Camera>(Orthographic, (float)16, (float)9, 80.0f, 0.1f, 1000.0f, XMFLOAT3(0, 0, -3.0f)));
	cams.push_back(std::make_shared<Camera>(Perspective, (float)windowWidth, (float)windowHeight, 100.0f, 0.1f, 1000.0f, XMFLOAT3(3, 0, -3.0f), XMFLOAT3(-0.2f, 0, 0)));
	cams.push_back(std::make_shared<Camera>(Perspective, (float)windowWidth, (float)windowHeight, 60.0f, 0.1f, 1000.0f, XMFLOAT3(-3, 0, -3.0f), XMFLOAT3(0.2f, 0, 0)));

	// Create the scene lights
	ambientLight = XMFLOAT3(0.2f, 0.2f, 0.2f);
	Light newLight = {};
	newLight.Type = LIGHT_TYPE_DIRECTIONAL;
	newLight.Direction = XMFLOAT3(0.0f, -0.3f, -1.0f);
	newLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	newLight.Intensity = 1.0f;
	lights.push_back(newLight);

	newLight = {};
	newLight.Type = LIGHT_TYPE_POINT;
	newLight.Position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	newLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	newLight.Intensity = 0.5f;
	newLight.Range = 5.0f;
	lights.push_back(newLight);

	newLight = {};
	newLight.Type = LIGHT_TYPE_POINT;
	newLight.Position = XMFLOAT3(3.0f, -1.0f, 0.0f);
	newLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	newLight.Intensity = 1.0f;
	newLight.Range = 10.0f;
	lights.push_back(newLight);

	// -- SHADOW MAPPING STUFF -- \\

	shadowMapRes = 1024;
	// Create Shadow Texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapRes; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowMapRes; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	// Create the light proj matrix
	float lightProjectionSize = 35.0f;
	XMStoreFloat4x4(&lightProj, XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f));

	// Create Rasterizer State for depth biasing
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	device->CreateRasterizerState(&shadowRastDesc, &shadowRS);

	// Create sampler state for shadow map
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	device->CreateSamplerState(&shadowSampDesc, &shadowSS);

	activeCam = cams[camIndex];
	
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();

	CreateGeometry();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// Make the shaders with SimpleShader (to see how to make them normally, check previous versions of this project)
	vertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader_PBR.cso").c_str());

	customPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"CustomPS.cso").c_str());

	skyVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader_Skybox.cso").c_str());
	skyPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader_Skybox.cso").c_str());

	// Create shadow vertex shader
	shadowVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VS_ScreenPosition.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Create the meshes
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), device, context));

	// Create the texture ptr for later use
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	// Create sampler description
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;     // for extreme angles
	samplerDesc.MaxAnisotropy = 16;                    // max anisotropic filtering quality
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;            // any mipmap range

	// Create the sampler state from the description
	device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	// Create materials
	std::vector<std::shared_ptr<Material>> mats;
	mats.push_back(std::make_shared<Material>(white, 0.0f, 0.0f, vertexShader, pixelShader));
	mats.push_back(std::make_shared<Material>(white, 0.0f, 0.0f, vertexShader, pixelShader));
	mats.push_back(std::make_shared<Material>(white, 0.0f, 0.0f, vertexShader, pixelShader));
	mats.push_back(std::make_shared<Material>(white, 0.0f, 0.0f, vertexShader, pixelShader));
	// weird material
	mats.push_back(std::make_shared<Material>(white, 0.1f, 0.0f, vertexShader, customPS));

	// Mat 0
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str(), 0, srv.GetAddressOf());
	mats[0]->AddTextureSRV("AlbedoMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_metal.png").c_str(), 0, srv.GetAddressOf());
	mats[0]->AddTextureSRV("MetalnessMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_normals.png").c_str(), 0, srv.GetAddressOf());
	mats[0]->AddTextureSRV("NormalMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str(), 0, srv.GetAddressOf());
	mats[0]->AddTextureSRV("RoughnessMap", srv);

	// Mat 1
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str(), 0, srv.GetAddressOf());
	mats[1]->AddTextureSRV("AlbedoMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_metal.png").c_str(), 0, srv.GetAddressOf());
	mats[1]->AddTextureSRV("MetalnessMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_normals.png").c_str(), 0, srv.GetAddressOf());
	mats[1]->AddTextureSRV("NormalMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str(), 0, srv.GetAddressOf());
	mats[1]->AddTextureSRV("RoughnessMap", srv);

	// Mat 2
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_albedo.png").c_str(), 0, srv.GetAddressOf());
	mats[2]->AddTextureSRV("AlbedoMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_metal.png").c_str(), 0, srv.GetAddressOf());
	mats[2]->AddTextureSRV("MetalnessMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str(), 0, srv.GetAddressOf());
	mats[2]->AddTextureSRV("NormalMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_roughness.png").c_str(), 0, srv.GetAddressOf());
	mats[2]->AddTextureSRV("RoughnessMap", srv);

	// Mat 3
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rough_albedo.png").c_str(), 0, srv.GetAddressOf());
	mats[3]->AddTextureSRV("AlbedoMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rough_normals.png").c_str(), 0, srv.GetAddressOf());
	mats[3]->AddTextureSRV("NormalMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rough_metal.png").c_str(), 0, srv.GetAddressOf());
	mats[3]->AddTextureSRV("MetalnessMap", srv);
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rough_roughness.png").c_str(), 0, srv.GetAddressOf());
	mats[3]->AddTextureSRV("RoughnessMap", srv);
	
	// Add default sampler and other maps for each material
	for (std::shared_ptr<Material> mat : mats)
		mat->AddSampler("SamplerOptions", samplerState);

	// Create the game objects
	gameObjects.push_back(new GameEntity(meshes[0], mats[0]));
	gameObjects.push_back(new GameEntity(meshes[1], mats[1]));

	gameObjects.push_back(new GameEntity(meshes[2], mats[2]));
	gameObjects.push_back(new GameEntity(meshes[2], mats[2]));
	gameObjects.push_back(new GameEntity(meshes[2], mats[2]));
	gameObjects.push_back(new GameEntity(meshes[2], mats[2]));

	gameObjects.push_back(new GameEntity(meshes[3], mats[0]));

	gameObjects.push_back(new GameEntity(meshes[0], mats[1]));
	gameObjects.push_back(new TerrainEntity(std::make_shared<Terrain>(500, 500, device, context), mats[3], XMFLOAT2(2.5f, 2.5f))); // cool terrain entity
	
	// Create the mirror manager (this creates the mirrors and sets up all the backend)
	mirrorManager = std::make_shared<MagicMirrorManager>(activeCam, device, context);

	// Call Init on all game entities in the world (if they have one)
	for (GameEntity* obj : gameObjects)
		obj->Init();
	mirrorManager->Init();

	gameObjects[0]->GetTransform()->SetPosition(-8, 0, 0);
	gameObjects[1]->GetTransform()->SetPosition(0, 6, 0);

	// right column
	gameObjects[2]->GetTransform()->SetPosition(4, -2, -2);
	gameObjects[3]->GetTransform()->SetPosition(4, 0, -2);
	// left column
	gameObjects[4]->GetTransform()->SetPosition(-4, -2, -2);
	gameObjects[5]->GetTransform()->SetPosition(-4, 0, -2);

	gameObjects[6]->GetTransform()->SetPosition(8, 0, 0);
	gameObjects[7]->GetTransform()->SetPosition(0, 4, 2);
	// ground texture
	gameObjects[8]->GetTransform()->MoveAbsolute(-250, -2, -250);
	gameObjects[8]->GetTransform()->SetScale(1.0f, 10.0f,1.0f);
	gameObjects[8]->SetTextureUniformScale(0.1f);

	// Create the skybox
	skybox = std::make_shared<Skybox>(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context), samplerState, device, context, skyVS, skyPS,
		FixPath(L"../../Assets/Textures/Clouds Blue/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Clouds Blue/back.png").c_str()
		);
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	for (std::shared_ptr<Camera> cam : cams)
		cam->UpdateProjectionMatrix((float)windowWidth, (float)windowHeight);

	mirrorManager->ResetMirrors(activeCam.get(), device);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	for (GameEntity* gameObj : gameObjects)
		gameObj->Update(deltaTime, context);

	activeCam->Update(deltaTime);
	activeCam->UpdateViewMatrix();

	// Update mirror view matrices & cams
	mirrorManager->Update(deltaTime, context, activeCam);

	// Update UI
	this->UpdateUI(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

void Game::UpdateUI(float deltaTime)
{
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	ImGui::ShowDemoWindow();

	ImGui::Begin("Stats");
	ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
	ImGui::Text("Window Width: %i", this->windowWidth);
	ImGui::Text("Window Height: %i", this->windowHeight);

	// Camera details
	if (ImGui::Button("Next Camera", ImVec2(150, 25)))
	{
		camIndex = (++camIndex) % cams.size();
		activeCam = cams[camIndex];
	}
	if (ImGui::Button("Previous Camera", ImVec2(150, 25)))
	{
		camIndex = (--camIndex) % cams.size();
		activeCam = cams[camIndex];
	}
	ImGui::End();

	// Game Object Inspector
	ImGui::Begin("Hierarchy");
	int currentTreeSize = 0;

	// Game Objects
	for (int i = 0; i < gameObjects.size(); i++)
	{
		if (ImGui::TreeNode((void*)(intptr_t)currentTreeSize, "Game Object %d", i))
		{
			ImGui::DragFloat3("Position: ", &gameObjects[i]->GetTransform()->GetPosition().x, 0.01f);
			ImGui::DragFloat3("Rotation: ", &gameObjects[i]->GetTransform()->GetPitchYawRoll().x, 0.01f);
			ImGui::DragFloat3("Scale: ", &gameObjects[i]->GetTransform()->GetScale().x, 0.01f);
			ImGui::TreePop();
		}
		currentTreeSize++;
	}

	// mirrors
	for (int i = 0; i < 2; i++)
	{
		if (ImGui::TreeNode((void*)(intptr_t)currentTreeSize, "Mirror %d", i))
		{
			ImGui::DragFloat3("Position: ", &mirrorManager->GetMirror(i)->GetTransform()->GetPosition().x, 0.01f);
			ImGui::DragFloat3("Rotation: ", &mirrorManager->GetMirror(i)->GetTransform()->GetPitchYawRoll().x, 0.01f);
			ImGui::DragFloat3("Scale: ", &mirrorManager->GetMirror(i)->GetTransform()->GetScale().x, 0.01f);
			ImGui::TreePop();
		}
		currentTreeSize++;
	}

	// Active Camera
	if (ImGui::TreeNode((void*)(intptr_t)currentTreeSize, "Active Camera (%d)", camIndex))
	{
		ImGui::Text("Position: %f, %f, %f", 
			activeCam->GetTransform().GetPosition().x,
			activeCam->GetTransform().GetPosition().y,
			activeCam->GetTransform().GetPosition().z);
		ImGui::Text("Rotation: %f, %f, %f",
			activeCam->GetTransform().GetPitchYawRoll().x,
			activeCam->GetTransform().GetPitchYawRoll().y,
			activeCam->GetTransform().GetPitchYawRoll().z);
		ImGui::Text("Field of View: %f",
			activeCam->fov);
		ImGui::TreePop();
	}
	currentTreeSize++;
	
	// Lights
	for (int i = 0; i < lights.size(); i++)
	{
		if (ImGui::TreeNode((void*)(intptr_t)currentTreeSize, "Light %d", i))
		{
			switch (lights[i].Type)
			{
			case LIGHT_TYPE_DIRECTIONAL:
				ImGui::Text("Type: Directional");
				ImGui::DragFloat3("Direction: ", &lights[i].Direction.x, 0.01f);
				break;
			case LIGHT_TYPE_POINT:
				ImGui::Text("Type: Point");
				ImGui::DragFloat3("Position: ", &lights[i].Position.x, 0.01f);
				break;
			case LIGHT_TYPE_SPOT:
				ImGui::Text("Type: Spot");
				ImGui::DragFloat3("Position: ", &lights[i].Position.x, 0.01f);
				ImGui::DragFloat3("Direction: ", &lights[i].Direction.x, 0.01f);
				break;
			}
			ImGui::TreePop();
		}
		currentTreeSize++;
	}

	// Shadow map
	ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));

	ImGui::End();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f }; // dark grey
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Clear the Shadow depth buffer
		context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	
	// Bind shadow map to render target view
	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());
	context->RSSetState(shadowRS.Get()); // set rasterizer state for depth biasing

	// Disable pixel processing for shadow map
	context->PSSetShader(0, 0, 0);

	// Change viewport resolution to match shadow map resolution
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapRes;
	viewport.Height = (float)shadowMapRes;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Set to basic VS and render entities
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", lightView);
	shadowVS->SetMatrix4x4("projection", lightProj);
	for (auto& e : gameObjects)
	{
		shadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		e->GetMesh()->Draw();
	}
	context->RSSetState(0); // disable depth biasing state

	// Reset the pipeline
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());

	// update view matrix (in the future this will be redone so it's only recalculated when the light transform changes)
	XMVECTOR lightDir = XMLoadFloat3(&lights[0].Direction);
	XMStoreFloat4x4(&lightView, XMMatrixLookToLH(
		-lightDir * 20, // Position: "Backing up" 20 units from origin
		lightDir, // Direction: light's direction
		XMVectorSet(0, 1, 0, 0))); // Up: World up vector (Y axis)

	// Render Game entities
	for (GameEntity* gameObject : gameObjects)
	{
		std::shared_ptr<SimplePixelShader> ps = gameObject->GetMaterial()->GetPS();
		std::shared_ptr<SimpleVertexShader> vs = gameObject->GetMaterial()->GetVS();
		vs->SetMatrix4x4("lightView", lightView);
		vs->SetMatrix4x4("lightProjection", lightProj);

		ps->SetData("lights",                         // name of the lights array in shader
			&lights[0],                               // address of the data to set
			sizeof(Light) * (int)lights.size());      // size of the data (whole struct) to set
		ps->SetFloat3("ambient", ambientLight);
		ps->SetShaderResourceView("ShadowMap", shadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSS);
		gameObject->Draw(context, activeCam);
		ps->SetShaderResourceView("ShadowMap", 0);
		ps->SetSamplerState("ShadowSampler", 0);
	}

	// Render the skybox
	skybox->Draw(context, activeCam);

	// Draw mirrors & update mirror maps, draw all objects through mirrors
	mirrorManager->Draw(context, activeCam, gameObjects, skybox, lights, ambientLight);

	// Render the UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}