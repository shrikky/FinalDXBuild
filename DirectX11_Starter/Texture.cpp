#include "Texture.h"



Texture::Texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename)
{
	
	CreateWICTextureFromFile(
		device,
		deviceContext,
		filename,
		0,
		&_texSRV);
	CreateWICTextureFromFile(device, deviceContext, filename, 0, &_texSRV);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &samplerState);
}


Texture::~Texture()
{
	_texSRV->Release();
}
