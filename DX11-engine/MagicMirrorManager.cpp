#include "MagicMirrorManager.h"
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

	// Create mirror render target
	D3D11_TEXTURE2D_DESC mirrorTextDesc = {};
	mirrorTextDesc.Width                = playerCam->viewDimensions.x;
	mirrorTextDesc.Height               = playerCam->viewDimensions.y;
	mirrorTextDesc.MipLevels            = 1;
	mirrorTextDesc.ArraySize            = 1;
	mirrorTextDesc.Format               = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
}

void MagicMirrorManager::Init()
{
	mirrors[1].GetTransform()->SetPosition(0, 3, 0);
}

void MagicMirrorManager::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, shared_ptr<Camera> camPtr)
{
	// Set mirror 2 cam in mirror 2 space, which is player cam in mirror 1 space but negated
	XMVECTOR mirror0PosVec = XMLoadFloat3(&mirrors[0].GetTransform()->GetPosition());
	XMVECTOR mirror1PosVec = XMLoadFloat3(&mirrors[1].GetTransform()->GetPosition());
	XMVECTOR mirrorCamPosOffset = XMVectorSubtract(mirror0PosVec,
		XMLoadFloat3(&camPtr->GetTransform().GetPosition())); // cam -> mirror

	XMVECTOR mirrorCamPosWorld = XMVectorAdd(mirror1PosVec, mirrorCamPosOffset);
	XMStoreFloat3(&mirrorCamPositions[1], mirrorCamPosWorld);

	// Get the axis of rotation between look-in mirror forward and cam's NEGATED forward
	XMVECTOR mirrorForward = XMLoadFloat3(&mirrors[0].GetTransform()->GetForward());
	XMVECTOR camForwadNeg = XMVectorMultiply(XMLoadFloat3(&camPtr->GetTransform().GetForward()), XMVectorSet(-1, 1, -1, 1));
	XMVECTOR axis = XMVector3Cross(mirrorForward, camForwadNeg);

	// Get the angle from the dot product
	float angleRad;
	XMStoreFloat(&angleRad, XMVector3Dot(mirrorForward, camForwadNeg));

	// Calculate result of mirror forward rotated angleRad radians around axis
	XMVECTOR newForward;
	if (abs(angleRad) == 1) // if 0 or 180 deg, don't use axis as it will have length 0
		newForward = angleRad == 1 ? mirrorForward : XMVectorScale(mirrorForward, -1);
	else
		newForward = XMVector3Rotate(mirrorForward, XMQuaternionRotationAxis(axis, acosf(angleRad)));

	// Then, use XMMatrixLookToLH() to create mirror cam's new view matrix
	XMMATRIX view = XMMatrixLookToLH(
		mirrorCamPosWorld,                              // mirror camera position in world space
		newForward,            // mirror Forward rotated by angle-axis quaternion
		XMLoadFloat3(&mirrors[1].GetTransform()->GetUp())); // mirror Up axis

	// Store matrix for sending to PS in Draw()
	XMStoreFloat4x4(&mirrorViews[1], view);

	// Set camera's near clip depth to farthest vertex in mirror mesh
	XMFLOAT4X4 mirrorWorld = mirrors[1].GetTransform()->GetWorldMatrix();
	XMVECTOR mirrorWorldZCol = XMVectorSet(mirrorWorld._31, mirrorWorld ._32, mirrorWorld ._33, mirrorWorld ._34);

	float newNear = camPtr->nearClip;
	for (Vertex& v : mirrors[1].GetMesh()->vertices)
	{
		float z;
		XMVECTOR posVec = XMLoadFloat3(&v.Position);
		XMStoreFloat(&z, XMVector3Dot(posVec, XMVector3Transform(mirrorWorldZCol, view)));
		if (z > newNear) newNear = z;
	}
	std::cout << newNear << std::endl;
	// Set this mirror's projection matrix w/ new near clip
	XMStoreFloat4x4(&mirrorProjs[1], XMMatrixPerspectiveFovLH(
		camPtr->fov * (3.14159f / 180.0f), 
		camPtr->viewDimensions.x / camPtr->viewDimensions.y, 
		newNear, camPtr->farClip));
}

void MagicMirrorManager::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	shared_ptr<Camera> camPtr, vector<GameEntity*> gameObjects)
{
	// Grab the original render targets for rebinding later
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> tempRender;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> tempDepth;
	context->OMGetRenderTargets(1, tempRender.GetAddressOf(), tempDepth.GetAddressOf());

	// Draw the mirrors (not to the viewport, but to the mirror texture)
	const float black[4] = { 0, 0, 0, 1 };
	context->ClearRenderTargetView(mirrorTarget.Get(), black);
	context->OMSetRenderTargets(1, mirrorTarget.GetAddressOf(), 0);

	for (MagicMirror& m : mirrors)
		m.Draw(context, camPtr);

	// rebind to the original render target
	context->OMSetRenderTargets(1, tempRender.GetAddressOf(), tempDepth.Get());

	// Re-render all game entities through the mirror
	for (GameEntity* gameObj : gameObjects)
	{
		shared_ptr<Material> mat = gameObj->GetMaterial();
		shared_ptr<SimplePixelShader> tempPS = mat->GetPS();
		mat->SetPS(mirrorViewPS); // set to mirror pixel shader for drawing through mirror
		Transform* trans = gameObj->GetTransform();

		// Do any routine prep work for the material's shaders (i.e. loading stuff)
		mat->PrepareMaterial();

		std::shared_ptr<SimpleVertexShader> vs = mat->GetVS();
		std::shared_ptr<SimplePixelShader> ps = mat->GetPS();

		vs->SetMatrix4x4("world", trans->GetWorldMatrix());
		vs->SetMatrix4x4("worldInvTranspose", trans->GetWorldInverseTransposeMatrix());
		vs->SetMatrix4x4("view", mirrorViews[1]);
		vs->SetMatrix4x4("projection", mirrorProjs[1]);

		vs->CopyAllBufferData(); // Adjust “vs” variable name if necessary

		ps->SetFloat4("colorTint", mat->GetColor());
		ps->SetFloat("roughness", mat->GetRoughness());
		ps->SetFloat3("cameraPosition", mirrorCamPositions[1]);
		ps->SetFloat("textureScale", textureScale);
		ps->SetFloat2("mirrorMapDimensions", camPtr->viewDimensions);

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
		mat->SetPS(tempPS); // reset back to original pixel shader
	}
}

// Get one of the mirrors (index 0 or 1)
MagicMirror* MagicMirrorManager::GetMirror(int index)
{
	if (index < 0 || index > 1) return 0;
	return &mirrors[index];
}