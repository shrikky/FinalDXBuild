#pragma once
#include "SimpleShader.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "DirectXGameCore.h"

using namespace DirectX;
class Material
{
public:
	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11SamplerState* _sampler;
	ID3D11RasterizerState*		_rasState;
	ID3D11DepthStencilState*	_depthState;
	ID3D11ShaderResourceView* _texSRV = 0;
	ID3D11ShaderResourceView* _texSRV1 = 0;
	ID3D11ShaderResourceView* _nMapSRV = 0;
	ID3D11ShaderResourceView* _dMapSRV = 0;
	ID3D11ShaderResourceView* _skySRV = 0;
	bool isSkybox = false;

	Material(SimpleVertexShader** vert, SimplePixelShader** pix);
	Material(SimpleVertexShader** vert, SimplePixelShader** pix, ID3D11Device** device, ID3D11DeviceContext** deviceContext, ID3D11SamplerState** sampler, ID3D11ShaderResourceView** texSRV,  const wchar_t* texture, ID3D11ShaderResourceView** nMap = NULL, const wchar_t* normalMap = 0,ID3D11ShaderResourceView** dMap = NULL,  const wchar_t* depthMap = 0);
	void Skybox(SimpleVertexShader** vert, SimplePixelShader** pix, ID3D11Device** device, ID3D11DeviceContext** deviceContext, ID3D11SamplerState** sampler, ID3D11ShaderResourceView** texSRV, ID3D11RasterizerState** rasState, ID3D11DepthStencilState** depthState, const wchar_t* texture);
	void UpdateShaderResources();
	~Material();
};

