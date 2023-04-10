#include "MagicMirrorManager.h"
#include "Helpers.h"

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
	mirrorTextDesc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
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

void MagicMirrorManager::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, shared_ptr<Camera> camPtr)
{
	// Set mirror 2 cam in mirror 2 space, which is player cam in mirror 1 space but negated
	XMVECTOR mirrorPosVec = XMLoadFloat3(&mirrors[0].GetTransform()->GetPosition());
	XMVECTOR mirrorCamPosOffset = XMVectorSubtract(mirrorPosVec,
		XMLoadFloat3(&camPtr->GetTransform().GetPosition())); // cam -> mirror

	XMVECTOR mirrorCamPosWorld = XMVectorAdd(mirrorPosVec, mirrorCamPosOffset);
	XMStoreFloat3(&mirrorCamPositions[0], mirrorCamPosWorld);

	// Get the axis of rotation between mirror forward and cam's NEGATED forward
	XMVECTOR mirrorForward = XMLoadFloat3(&mirrors[0].GetTransform()->GetForward());
	XMVECTOR axis = XMVector3Cross(mirrorForward,
		XMVectorScale(XMLoadFloat3(&camPtr->GetTransform().GetForward()), -1.0f)
	);

	// Get the angle from the axis length
	float angleRad;
	XMStoreFloat(&angleRad, XMVector3Length(axis));
	angleRad = asinf(angleRad);

	// Calculate result of mirror forward rotated angleRad radians around axis
	// Then, use XMMatrixLookToLH() to create mirror cam's new view matrix
	XMVECTOR rotQuat = XMQuaternionRotationAxis(axis, angleRad);
	XMMATRIX view = XMMatrixLookToLH(
		mirrorCamPosWorld,                              // mirror camera position in world space
		XMVector3Rotate(mirrorForward, rotQuat),            // mirror Forward rotated by angle-axis quaternion
		XMLoadFloat3(&mirrors[0].GetTransform()->GetUp())); // mirror Up axis

	// Store matrix for sending to PS in Draw()
	XMStoreFloat4x4(&mirrorViews[0], view);

	// Set camera's near clip depth to farthest vertex in mirror mesh
	XMFLOAT4X4 mirrorWorld = mirrors[0].GetTransform()->GetWorldMatrix();
	XMVECTOR mirrorWorldZCol = XMVectorSet(mirrorWorld._31, mirrorWorld ._32, mirrorWorld ._33, mirrorWorld ._34);

	float newNear = camPtr->nearClip;
	for (Vertex& v : mirrors[0].GetMesh()->vertices)
	{
		float z;
		XMVECTOR posVec = XMVectorSet(v.Position.x, v.Position.y, v.Position.z, 1);
		XMStoreFloat(&z, XMVector3Dot(posVec, XMVector3Transform(mirrorWorldZCol, view)));
		if (z > newNear) newNear = z;
	}

	// Set this mirror's projection matrix w/ new near clip
	XMStoreFloat4x4(&mirrorProjs[0], XMMatrixPerspectiveFovLH(
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
		vs->SetMatrix4x4("view", mirrorViews[0]);
		vs->SetMatrix4x4("projection", mirrorProjs[0]);

		vs->CopyAllBufferData(); // Adjust “vs” variable name if necessary

		ps->SetFloat4("colorTint", mat->GetColor());
		ps->SetFloat("roughness", mat->GetRoughness());
		ps->SetFloat3("cameraPosition", mirrorCamPositions[0]);
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