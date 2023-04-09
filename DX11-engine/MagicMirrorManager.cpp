#include "MagicMirrorManager.h"
#include "Helpers.h"

using namespace std;
using namespace DirectX;

MagicMirrorManager::MagicMirrorManager(shared_ptr<Camera> playerCam, 
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context) : GameEntity()
{
	this->playerCam = playerCam;

	// Create mirror shaders
	shared_ptr<SimpleVertexShader> mirrorVS = make_shared<SimpleVertexShader>(device, context, FixPath(L"VS_MagicMirror.cso").c_str());
	shared_ptr<SimplePixelShader> mirrorPS = make_shared<SimplePixelShader>(device, context, FixPath(L"PS_MagicMirror.cso").c_str());

	// Create the mirrors
	for (int i = 0; i < 2; i++)
	{
		shared_ptr<Material> mirrorMat = make_shared<Material>(XMFLOAT4(0, 0, 0 ,0), 0, mirrorVS, mirrorPS);
		Vertex verts[4] = 
		{
			{{ -1, 1, 0 }},
			{{1, 1, 0}},
			{{1, -1, 0}},
			{{-1, -1, 0}}
		};
		int indices[6] = { 0, 1, 2, 0, 2, 3 };
		mirrors[i] = MagicMirror(make_shared<Mesh>(verts, 4, indices, 6, device, context), mirrorMat);
	}

	// Create mirror render target
	D3D11_TEXTURE2D_DESC mirrorTargetDesc = {};
	mirrorTargetDesc.Width = playerCam->viewDimensions.x;
	mirrorTargetDesc.Height = playerCam->viewDimensions.y;
	mirrorTargetDesc.MipLevels = 1;
	mirrorTargetDesc.ArraySize = 1;
	mirrorTargetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	mirrorTargetDesc.Usage = D3D11_USAGE_DEFAULT;
	mirrorTargetDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	mirrorTargetDesc.CPUAccessFlags = 0;
	mirrorTargetDesc.MiscFlags = 0;
	mirrorTargetDesc.SampleDesc.Count = 1;
	mirrorTargetDesc.SampleDesc.Quality = 0;

	device->CreateTexture2D(&mirrorTargetDesc, 0, mirrorTextures.GetAddressOf());
	device->CreateRenderTargetView(mirrorTextures.Get(), 0, mirrorTarget.GetAddressOf());
}

void MagicMirrorManager::Update(float deltaTime, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Set mirror 2 cam in mirror 2 space, which is player cam in mirror 1 space but negated
	XMVECTOR mirrorPosVec = XMLoadFloat3(&mirrors[0].GetTransform()->GetPosition());
	XMVECTOR mirrorCamPosOffset = XMVectorSubtract(mirrorPosVec,
		XMLoadFloat3(&playerCam->GetTransform().GetPosition())); // cam -> mirror
	
	// Get the axis of rotation between mirror forward and cam's NEGATED forward
	XMVECTOR mirrorForward = XMLoadFloat3(&mirrors[0].GetTransform()->GetForward());
	XMVECTOR axis = XMVector3Cross(mirrorForward,
		XMVectorScale(XMLoadFloat3(&playerCam->GetTransform().GetForward()), -1.0f)
	);

	// Get the angle from the axis length
	float angleRad;
	XMStoreFloat(&angleRad, XMVector3Length(axis));
	angleRad = asinf(angleRad);

	// Calculate result of mirror forward rotated angleRad radians around axis
	// Then, use XMMatrixLookToLH() to create mirror cam's new view matrix
	XMVECTOR rotQuat = XMQuaternionRotationAxis(axis, angleRad);
	XMMATRIX view = XMMatrixLookToLH(
		XMVectorAdd(mirrorPosVec, mirrorCamPosOffset),       // mirror camera position in world space
		XMVector3Rotate(mirrorForward, rotQuat),             // mirror Forward rotated by angle-axis quaternion
		XMLoadFloat3(&mirrors[0].GetTransform()->GetUp()));  // mirror Up axis

	// Set camera's near clip depth to farthest vertex in mirror mesh
	XMFLOAT4X4 mirrorWorld = mirrors[0].GetTransform()->GetWorldMatrix();
	XMVECTOR mirrorWorldZCol = XMVectorSet(mirrorWorld._31, mirrorWorld ._32, mirrorWorld ._33, mirrorWorld ._34);

	float newNear = mirrorCams[0]->nearClip;
	for (Vertex& v : mirrors[0].GetMesh()->vertices)
	{
		float z;
		XMVECTOR posVec = XMVectorSet(v.Position.x, v.Position.y, v.Position.z, 1);
		XMStoreFloat(&z, XMVector3Dot(posVec, XMVector3Transform(mirrorWorldZCol, view)));
		if (z > newNear) newNear = z;
	}
	mirrorCams[0]->nearClip = newNear;
}

void MagicMirrorManager::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, shared_ptr<Camera> camPtr)
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
}

// Get one of the mirrors (index 0 or 1)
MagicMirror* MagicMirrorManager::GetMirror(int index)
{
	if (index < 0 || index > 1) return;
	return &mirrors[index];
}