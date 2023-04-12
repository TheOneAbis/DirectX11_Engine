﻿#include "MagicMirrorManager.h"
#include "Helpers.h"
#include <iostream>

using namespace std;
using namespace DirectX;

MagicMirrorManager::MagicMirrorManager(shared_ptr<Camera> playerCam, 
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) : GameEntity()
{
	// Create mirror shaders
	shared_ptr<SimpleVertexShader> mirrorVS = make_shared<SimpleVertexShader>(device, context, FixPath(L"VS_MagicMirror.cso").c_str());
	shared_ptr<SimplePixelShader> mirrorPS = make_shared<SimplePixelShader>(device, context, FixPath(L"PS_MagicMirror.cso").c_str());
	mirrorViewPS = make_shared<SimplePixelShader>(device, context, FixPath(L"PS_MagicMirrorView.cso").c_str());
	skyboxMirrorPS = make_shared<SimplePixelShader>(device, context, FixPath(L"PS_SkyboxMirror.cso").c_str());

	shared_ptr<Material> mirrorMat = make_shared<Material>(XMFLOAT4(0, 0, 0, 0), 0, mirrorVS, mirrorPS);
	Vertex verts[4] =
	{
		{{ -1, 1, 0 }},
		{{1, 1, 0}},
		{{1, -1, 0}},
		{{-1, -1, 0}}
	};
	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };

	// Create the mirrors
	for (int i = 0; i < 2; i++)
		mirrors[i] = MagicMirror(make_shared<Mesh>(verts, 4, indices, 6, device, context), mirrorMat);

	// Create mirror render target and SRV
	D3D11_TEXTURE2D_DESC mirrorTextDesc = {};
	mirrorTextDesc.Width                = playerCam->viewDimensions.x;
	mirrorTextDesc.Height               = playerCam->viewDimensions.y;
	mirrorTextDesc.MipLevels            = 1;
	mirrorTextDesc.ArraySize            = 1;
	mirrorTextDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
	mirrorTextDesc.Usage                = D3D11_USAGE_DEFAULT;
	mirrorTextDesc.BindFlags            = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	mirrorTextDesc.CPUAccessFlags       = 0;
	mirrorTextDesc.MiscFlags            = 0;
	mirrorTextDesc.SampleDesc.Count     = 1;
	mirrorTextDesc.SampleDesc.Quality   = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mirrorTextures;

	device->CreateTexture2D(&mirrorTextDesc, 0, mirrorTextures.GetAddressOf());
	device->CreateRenderTargetView(mirrorTextures.Get(), 0, mirrorTarget.GetAddressOf());
	device->CreateShaderResourceView(mirrorTextures.Get(), 0, mirrorSRV.GetAddressOf());

	// Create mirror-unique depth buffers
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = playerCam->viewDimensions.x;
	depthStencilDesc.Height = playerCam->viewDimensions.y;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	// Create the depth buffer texture resources for both mirrors
	for (int i = 0; i < 2; i++)
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthBufferTexture;
		device->CreateTexture2D(&depthStencilDesc, 0, depthBufferTexture.GetAddressOf());

		// If texture successfully created, create the DSV
		if (depthBufferTexture != 0)
			device->CreateDepthStencilView(depthBufferTexture.Get(), 0, mirrorDSVs[i].GetAddressOf());
	}
}

void MagicMirrorManager::Init()
{
	mirrors[1].GetTransform()->SetPosition(0.5f, 6.0f, 1.0f);
	mirrors[1].GetTransform()->Rotate(0, 0.707f, 0);
}

void MagicMirrorManager::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, shared_ptr<Camera> camPtr)
{
	mirrors[0].GetTransform()->Rotate(0, 0, 0);
	mirrors[1].GetTransform()->Rotate(0, 0, 0);

	for (int i = 0; i < 2; i++)
	{
		// -- CALCULATE MIRROR CAM POSITION
		// Get player cam in mirror-in space, negate x and z, then put it back in world space relative to mirror-out
		XMMATRIX mirrorInWorld = XMLoadFloat4x4(&mirrors[i].GetTransform()->GetWorldMatrix());
		XMVECTOR d = XMMatrixDeterminant(mirrorInWorld); // for calculating the inverse world
		XMMATRIX mirrorOutWorld = XMLoadFloat4x4(&mirrors[(i + 1) % 2].GetTransform()->GetWorldMatrix());

		XMVECTOR camPosMirrorOut = XMLoadFloat3(&camPtr->GetTransform().GetPosition()); // cam in world space
		camPosMirrorOut = XMVector3Transform(camPosMirrorOut, XMMatrixInverse(&d, mirrorInWorld)); // cam in mirror-in space
		camPosMirrorOut = XMVectorMultiply(camPosMirrorOut, XMVectorSet(-1, 1, -1, 1)); // negate x and z
		camPosMirrorOut = XMVector3Transform(camPosMirrorOut, mirrorOutWorld); // consider cam to be in mirror-out space now, and put it back in world space
		XMStoreFloat3(&mirrorCamPositions[(i + 1) % 2], camPosMirrorOut);

		// -- CALCULATE MIRROR CAM ROTATION
		XMVECTOR camQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&camPtr->GetTransform().GetPitchYawRoll()));
		XMVECTOR mirrorInQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&mirrors[i].GetTransform()->GetPitchYawRoll()));
		XMVECTOR mirrorOutQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&mirrors[(i + 1) % 2].GetTransform()->GetPitchYawRoll()));

		camQuat = XMQuaternionMultiply(camQuat, XMQuaternionInverse(mirrorInQuat));
		XMVECTOR newForward = XMVectorMultiply(XMVector3Rotate(XMVectorSet(0, 0, -1, 0), camQuat), XMVectorSet(1, -1, 1, 1));
		camQuat = XMQuaternionMultiply(camQuat, mirrorOutQuat);

		newForward = XMVector3Rotate(newForward, mirrorOutQuat);
		XMVECTOR newUp = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), camQuat); //stupid dumb stupid idiot not workin shit buttcock

		// Then, use XMMatrixLookToLH() to create mirror cam's new view matrix
		XMStoreFloat4x4(&mirrorViews[(i + 1) % 2], XMMatrixLookToLH(camPosMirrorOut, newForward, newUp));

		// Set this mirror's projection matrix w/ new near clip
		XMStoreFloat4x4(&mirrorProjs[(i + 1) % 2], XMMatrixPerspectiveFovLH(
			camPtr->fov * (3.14159f / 180.0f),
			camPtr->viewDimensions.x / camPtr->viewDimensions.y,
			camPtr->nearClip, camPtr->farClip));
	}
}


void MagicMirrorManager::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	shared_ptr<Camera> camPtr, 
	vector<GameEntity*> gameObjects, 
	std::shared_ptr<Skybox> skybox,
	vector<Light> lights, 
	XMFLOAT3 ambientColor)
{
	const float black[4] = { 0, 0, 0, 0 }; // dark grey

	// Grab the original render targets for rebinding later
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> tempRender;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> tempDepth;
	context->OMGetRenderTargets(1, tempRender.GetAddressOf(), tempDepth.GetAddressOf());

	for (int i = 0; i < 2; i++)
	{
		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(mirrorDSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Draw the mirrors (not to the viewport, but to the mirror texture)
		context->ClearRenderTargetView(mirrorTarget.Get(), black);
		context->OMSetRenderTargets(1, mirrorTarget.GetAddressOf(), tempDepth.Get()); // keep original DSV for setting the correct white pixels
		mirrors[i].Draw(context, camPtr);

		// rebind to the original render target and use the 
		// mirror's DSV now to draw objects through the mirror
		context->OMSetRenderTargets(1, tempRender.GetAddressOf(), mirrorDSVs[i].Get());

		// Re-render all game entities through the mirror
		for (GameEntity* gameObj : gameObjects)
		{
			shared_ptr<Material> mat = gameObj->GetMaterial();
			shared_ptr<SimplePixelShader> tempPS = mat->GetPS();
			mat->SetPS(mirrorViewPS); // set to mirror pixel shader for drawing through mirror
			Transform* trans = gameObj->GetTransform();

			std::shared_ptr<SimpleVertexShader> vs = mat->GetVS();
			std::shared_ptr<SimplePixelShader> ps = mat->GetPS();

			ps->SetData("lights",
				&lights[0],
				sizeof(Light) * (int)lights.size());
			ps->SetFloat3("ambientColor", ambientColor);

			// Do any routine prep work for the material's shaders (i.e. loading stuff)
			mat->PrepareMaterial();

			vs->SetMatrix4x4("world", trans->GetWorldMatrix());
			vs->SetMatrix4x4("worldInvTranspose", trans->GetWorldInverseTransposeMatrix());
			vs->SetMatrix4x4("view", mirrorViews[(i + 1) % 2]);
			vs->SetMatrix4x4("projection", mirrorProjs[(i + 1) % 2]);

			vs->CopyAllBufferData(); // Adjust “vs” variable name if necessary

			ps->SetFloat4("colorTint", mat->GetColor());
			ps->SetFloat("roughness", mat->GetRoughness());
			ps->SetFloat3("cameraPosition", mirrorCamPositions[(i + 1) % 2]);
			ps->SetFloat("textureScale", gameObj->GetTextureUniformScale());
			ps->SetFloat2("mirrorMapDimensions", camPtr->viewDimensions);
			ps->SetFloat3("mirrorNormal", mirrors[(i + 1) % 2].GetTransform()->GetForward());
			ps->SetFloat3("mirrorPos", mirrors[(i + 1) % 2].GetTransform()->GetPosition());

			// Set pixel shader mirror map
			ps->SetShaderResourceView("MirrorMap", mirrorSRV);

			ps->CopyAllBufferData();

			// Set the shaders for this entity
			vs->SetShader();
			ps->SetShader();

			// Draw the mesh
			gameObj->GetMesh()->Draw();

			// reset the SRV's and samplers for the next time so shader is fresh for a different material
			mat->ResetTextureData();
			ps->SetShaderResourceView("MirrorMap", 0);
			mat->SetPS(tempPS); // reset back to original pixel shader
		}

		// Draw the skybox through the mirror
		skyboxMirrorPS->SetFloat2("mirrorMapDimensions", camPtr->viewDimensions);
		skyboxMirrorPS->SetShaderResourceView("MirrorMap", mirrorSRV);
		shared_ptr<SimplePixelShader> skyTempPS = skybox->GetPS();
		skybox->SetPS(skyboxMirrorPS);
		skybox->Draw(context, mirrorViews[(i + 1) % 2], mirrorProjs[(i + 1) % 2]);
		skybox->SetPS(skyTempPS);
		skyboxMirrorPS->SetShaderResourceView("MirrorMap", 0);
	}

	// Set back to original DSV
	context->OMSetRenderTargets(1, tempRender.GetAddressOf(), tempDepth.Get());
}

// Get one of the mirrors (index 0 or 1)
MagicMirror* MagicMirrorManager::GetMirror(int index)
{
	if (index < 0 || index > 1) return 0;
	return &mirrors[index];
}