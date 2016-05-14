#pragma once

#include <DirectXMath.h>
#include "DirectXGameCore.h"
#include "SimpleShader.h"
#include "GameObject.h"
#include "Material.h"
#include "Camera.h"
#include "Mesh.h"
#include <math.h>
#include "Lights.h"
#include "imgui_impl_dx11.h"
#include <imgui.h>
#include <d3d11.h>
#include <vector>
#include "SkyBox.h"
// Include run-time memory checking in debug builds, so 
// we can be notified of memory leaks
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// --------------------------------------------------------
// Game class which extends the base DirectXGameCore class
// --------------------------------------------------------
class MyDemoGame : public DirectXGameCore
{
public:
	MyDemoGame(HINSTANCE hInstance);
	~MyDemoGame();

	// Overrides for base level methods
	bool Init();
	void OnResize();
	void UpdateScene(float deltaTime, float totalTime);
	void DrawScene(float deltaTime, float totalTime);

	// For handing mouse input
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	// Initialization for our "game" demo - Feel free to
	// expand, alter, rename or remove these once you
	// start doing something more advanced!
	void LoadShaders(); 
	void CreateGeometry();
	void CreateMatrices();
	void MakePostProcessContent(D3D11_TEXTURE2D_DESC& tDesc, D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, ID3D11Texture2D*& ppTexture, ID3D11RenderTargetView*& ppRTV, ID3D11ShaderResourceView*& ppSRV);

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	SimplePixelShader* normalMappingPS;
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;
	SimpleVertexShader* ppVS;
	SimplePixelShader* ppPS;
	SimplePixelShader* mergePS;
	SimplePixelShader* brtPS;


	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
	POINT p;

	// Meshes
	Mesh* _cube;
	Mesh* _cube2;
	Mesh* _helix;
	Mesh* sbCube;
	std::vector<Mesh*> meshes;
	// GameObjects
	std::vector<GameObject*> gameObjects;
	GameObject* helixGameObject;
	GameObject* skyBoxCube;

	ID3D11BlendState* blendState;

	//Skyboxes
	SkyBox* _skybox;
	//SRV
	ID3D11ShaderResourceView* texSRV = 0;
	ID3D11ShaderResourceView* nMapSRV = 0;
	ID3D11ShaderResourceView* helixTexSRV = 0;
	ID3D11ShaderResourceView* dMapSRV = 0;
	ID3D11ShaderResourceView* skySRV = 0;
	ID3D11ShaderResourceView* mSRV = 0;
	ID3D11ShaderResourceView* bpSRV = 0;
	ID3D11ShaderResourceView* brtSRV = 0;

	//Textures
	ID3D11Texture2D* mTexture;
	ID3D11Texture2D* brtTexture;
	ID3D11Texture2D* bpTexture;


	//RenderTargetView
	ID3D11RenderTargetView* mRTV;
	ID3D11RenderTargetView* bpRTV;
	ID3D11RenderTargetView* brtpRTV;

	// Samplers
	ID3D11SamplerState* samplerState;
	ID3D11RasterizerState*		rasState;
	ID3D11DepthStencilState*	depthState;

	//Materials
	Material* _helixMaterial;
	Material* _NormalMapMat;
	Material* skyBoxMaterial;

	//Descriptions
	D3D11_TEXTURE2D_DESC mtDesc;
	D3D11_TEXTURE2D_DESC brttDesc;
	D3D11_TEXTURE2D_DESC blurtDesc;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	D3D11_RENDER_TARGET_VIEW_DESC brtvDesc;
	D3D11_RENDER_TARGET_VIEW_DESC blurrtvDesc;

	D3D11_SHADER_RESOURCE_VIEW_DESC brtsrvDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC blursrvDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC msrvDesc;


	Camera* myCamera;
	WPARAM btnState;

	DirectionLight directionLight;
	DirectionLight directionalLight2;
	PointLight pointLight;
	SpecularLight specularLight;

	std::vector<ID3D11ShaderResourceView*> srvContainer;
	bool show_test_window = false;
	bool show_another_window = false;
	ImVec4 clear_col = ImColor(114, 144, 154);
	 float r = 1.0f;
	 float g = 0.0f;
	 float b = 0.0f;

	 bool isBloom = false;
};
