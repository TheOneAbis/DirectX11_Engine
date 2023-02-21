#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

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

	camIndex = 0;
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

	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

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
	
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();

	// Create materials
	mat1 = std::make_shared<Material>(XMFLOAT4(1, 0, 1, 1), vertexShader, pixelShader);
	mat2 = std::make_shared<Material>(XMFLOAT4(0, 1, 1, 1), vertexShader, pixelShader);
	mat3 = std::make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vertexShader, pixelShader);

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

	//// Get size as the next multiple of 16 (instead of hardcoding a size here)
	//unsigned int size = sizeof(VertexShaderExternalData);
	//size = (size + 15) / 16 * 16; // This will work even if the struct size changes

	//// Describe the constant buffer
	//D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	//cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cbDesc.ByteWidth = size; // Must be a multiple of 16
	//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	// Create the constant buffer
	//device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());

	// Init the cameras
	cams.push_back(std::make_shared<Camera>(Perspective, (float)windowWidth, (float)windowHeight, 80.0f, 0.1f, 1000.0f, XMFLOAT3(0, 0, -3.0f)));
	cams.push_back(std::make_shared<Camera>(Orthographic, (float)windowWidth, (float)windowHeight, 80.0f, 0.1f, 1000.0f, XMFLOAT3(0, 0, -3.0f)));
	cams.push_back(std::make_shared<Camera>(Perspective, (float)windowWidth, (float)windowHeight, 100.0f, 0.1f, 1000.0f, XMFLOAT3(3, 0, -3.0f), XMFLOAT3(-0.2f, 0, 0)));
	cams.push_back(std::make_shared<Camera>(Perspective, (float)windowWidth, (float)windowHeight, 60.0f, 0.1f, 1000.0f, XMFLOAT3(-3, 0, -3.0f), XMFLOAT3(0.2f, 0, 0)));

	activeCam = cams[camIndex];
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
	//// BLOBs (or Binary Large OBjects) for reading raw data from external files
	//// - This is a simplified way of handling big chunks of external data
	//// - Literally just a big array of bytes read from a file
	//ID3DBlob* pixelShaderBlob;
	//ID3DBlob* vertexShaderBlob;

	//// Loading shaders
	////  - Visual Studio will compile our shaders at build time
	////  - They are saved as .cso (Compiled Shader Object) files
	////  - We need to load them when the application starts
	//{
	//	// Read our compiled shader code files into blobs
	//	// - Essentially just "open the file and plop its contents here"
	//	// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
	//	// - Note the "L" before the string - this tells the compiler the string uses wide characters
	//	D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
	//	D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

	//	// Create the actual Direct3D shaders on the GPU
	//	device->CreatePixelShader(
	//		pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
	//		pixelShaderBlob->GetBufferSize(),		// How big is that data?
	//		0,										// No classes in this shader
	//		pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

	//	device->CreateVertexShader(
	//		vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
	//		vertexShaderBlob->GetBufferSize(),		// How big is that data?
	//		0,										// No classes in this shader
	//		vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	//}

	//// Create an input layout 
	////  - This describes the layout of data sent to a vertex shader
	////  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	////  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	////  - Luckily, we already have that loaded (the vertex shader blob above)
	//{
	//	D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

	//	// Set up the first element - a position, which is 3 float values
	//	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
	//	inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
	//	inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

	//	// Set up the second element - a color, which is 4 more float values
	//	inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
	//	inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
	//	inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

	//	// Create the input layout, verifying our description against actual shader code
	//	device->CreateInputLayout(
	//		inputElements,							// An array of descriptions
	//		2,										// How many elements in that array?
	//		vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
	//		vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
	//		inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	//}

	// Make the shaders with SimpleShader
	vertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
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
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT3 defaultNormal = XMFLOAT3(0, 0, -1);
	XMFLOAT2 defaultUV = XMFLOAT2(0, 0);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), defaultNormal, defaultUV },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), defaultNormal, defaultUV },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), defaultNormal, defaultUV },
	};
	Vertex rectVertices[] =
	{
		{ XMFLOAT3(-0.9f, -0.4f, +0.0f), defaultNormal, defaultUV }, // top left
		{ XMFLOAT3(-0.4f, -0.4f, +0.0f), defaultNormal, defaultUV }, // top right
		{ XMFLOAT3(-0.4f, -0.9f, +0.0f), defaultNormal, defaultUV }, // bottom right
		{ XMFLOAT3(-0.9f, -0.9f, +0.0f), defaultNormal, defaultUV }  // bottom left
	};
	Vertex weirdVertices[] =
	{
		{ XMFLOAT3(+0.2f, +0.9f, +0.0f), defaultNormal, defaultUV },
		{ XMFLOAT3(+0.7f, +0.9f, +0.0f), defaultNormal, defaultUV },
		{ XMFLOAT3(+0.8f, +0.8f, +0.0f), defaultNormal, defaultUV },
		{ XMFLOAT3(+0.7f, +0.7f, +0.0f), defaultNormal, defaultUV },
		{ XMFLOAT3(+0.2f, +0.7f, +0.0f), defaultNormal, defaultUV },
		{ XMFLOAT3(+0.1f, +0.8f, +0.0f), defaultNormal, defaultUV }
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };
	unsigned int rectIndices[] = 
	{ 
		0, 1, 2, // tri 1
		2, 3, 0  // tri 2
	};
	unsigned int weirdIndices[] =
	{
		0, 1, 3, // tri 1
		1, 2, 3, // tri 2
		0, 3, 4, // tri 3
		0, 4, 5  // tri 4
	};

	// Create the meshes
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));

	// Create the game objects
	gameObjects.push_back(GameEntity(meshes[0], mat1));
	gameObjects.push_back(GameEntity(meshes[1], mat1));
	gameObjects.push_back(GameEntity(meshes[2], mat2));
	gameObjects.push_back(GameEntity(meshes[3], mat3));
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
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Do funny transformations on game objects
	gameObjects[0].GetTransform()->Rotate(0, 0, deltaTime);
	gameObjects[2].GetTransform()->SetScale(sinf(totalTime) + 1, 1, 1);
	gameObjects[1].GetTransform()->MoveAbsolute(deltaTime / 10, deltaTime / 10, 0);
	gameObjects[3].GetTransform()->Rotate(0, 0, deltaTime / 3);

	activeCam->Update(deltaTime);
	activeCam->UpdateViewMatrix();

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
	
	for (int i = 0; i < 6; i++)
	{
		if (ImGui::TreeNode((void*)(intptr_t)i, "Game Object %d", i))
		{
			ImGui::DragFloat3("Position: ", &gameObjects[i].GetTransform()->GetPosition().x, 0.01f);
			ImGui::DragFloat3("Rotation: ", &gameObjects[i].GetTransform()->GetPitchYawRoll().x, 0.01f);
			ImGui::DragFloat3("Scale: ", &gameObjects[i].GetTransform()->GetScale().x, 0.01f);
			ImGui::TreePop();
		}
	}
	if (ImGui::TreeNode((void*)(intptr_t)6, "Active Camera (%d)", camIndex))
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
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// Render Game entities
	for (GameEntity& gameObject : gameObjects)
		gameObject.Draw(context, activeCam);

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