#pragma once

#include "Mesh.h"

class Terrain : public Mesh
{
public:
	
	Terrain(unsigned int rows, unsigned int columns,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	void Draw() override;

	void UpdateVBO();

	DirectX::XMINT2 resolution;

private:

	bool updateVBO;

};