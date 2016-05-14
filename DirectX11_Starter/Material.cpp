#include "Material.h"


Material::Material(SimpleVertexShader** vert, SimplePixelShader** pix) {
	vertexShader = *vert;
	pixelShader = *pix;
}

Material::Material(SimpleVertexShader** vert, SimplePixelShader** pix, ID3D11Device** device, ID3D11DeviceContext** deviceContext, ID3D11SamplerState** sampler, ID3D11ShaderResourceView** texSRV, const wchar_t* texture, ID3D11ShaderResourceView** nMap, const wchar_t* normalMap, ID3D11ShaderResourceView** dMap, const wchar_t* depthMap) {
	vertexShader = *vert;
	pixelShader = *pix;
	_sampler = *sampler;
	
		CreateWICTextureFromFile(*device, *deviceContext, texture, 0, texSRV);
		_texSRV = *texSRV;

	if (normalMap) {
		
		CreateWICTextureFromFile(*device, *deviceContext, normalMap, 0, nMap);
		_nMapSRV = *nMap;
	}
	if (depthMap) {
		
		CreateWICTextureFromFile(*device, *deviceContext, depthMap, 0, dMap);
		_dMapSRV = *dMap;
	}


}

void Material::Skybox (SimpleVertexShader** vert, SimplePixelShader** pix, ID3D11Device** device, ID3D11DeviceContext** deviceContext, ID3D11SamplerState** sampler, ID3D11ShaderResourceView** texSRV, ID3D11RasterizerState** rasState, ID3D11DepthStencilState** depthState, const wchar_t* texture)
{
	vertexShader = *vert;
	pixelShader = *pix;
	_sampler = *sampler;
	isSkybox = true;
	_rasState = *rasState;
	_depthState = *depthState;
	CreateDDSTextureFromFile(*device, *deviceContext, texture, 0, texSRV);
	_skySRV = *texSRV;
	//pixelShader->SetShaderResourceView("skyTexture", *texSRV);
	//pixelShader->SetSamplerState("trillinear", samplerState);

}
void Material::UpdateShaderResources() {


	if (!isSkybox)
		pixelShader->SetShaderResourceView("diffuseTexture", _texSRV);
	else
		pixelShader->SetShaderResourceView("skyTexture", _skySRV);
	if (_nMapSRV)
		pixelShader->SetShaderResourceView("normalMap", _nMapSRV);
	if (_dMapSRV)
		pixelShader->SetShaderResourceView("depthMap", _dMapSRV);

		pixelShader->SetSamplerState("trillinear", _sampler);

}
Material::~Material()
{

}
