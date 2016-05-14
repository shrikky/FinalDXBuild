#pragma once
#include "WICTextureLoader.h"
#include "DirectXGameCore.h"

using namespace DirectX;
class Texture
{
public:
	ID3D11SamplerState* samplerState;
	ID3D11ShaderResourceView* _texSRV;
	Texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename);
	~Texture();
};

