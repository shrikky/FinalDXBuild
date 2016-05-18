#include "MyDemoGame.h"
#include "Vertex.h"
#include <iostream>
//#include "vld.h"
#include "DDSTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;


#pragma region Win32 Entry Point (WinMain)
// --------------------------------------------------------
// Win32 Entry Point - Where your program starts
// --------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Create the game object.
	MyDemoGame game(hInstance);
	
	// This is where we'll create the window, initialize DirectX, 
	// set up geometry and shaders, etc.
	if( !game.Init() )
		return 0;
	
	// All set to run the game loop
	return game.Run();
}

#pragma endregion

#pragma region Constructor / Destructor
// --------------------------------------------------------
// Base class constructor will set up all of the underlying
// fields, and then we can overwrite any that we'd like
// --------------------------------------------------------
MyDemoGame::MyDemoGame(HINSTANCE hInstance) 
	: DirectXGameCore(hInstance)
{
	// Set up a custom caption for the game window.
	// - The "L" before the string signifies a "wide character" string
	// - "Wide" characters take up more space in memory (hence the name)
	// - This allows for an extended character set (more fancy letters/symbols)
	// - Lots of Windows functions want "wide characters", so we use the "L"
	windowCaption = L"My Super Fancy GGP Game";

	// Custom window size - will be created by Init() later
	windowWidth = 1280;
	windowHeight = 720;
	shadowMapSize = 16384; // bigger shadow map
}

// --------------------------------------------------------
// Cleans up our DirectX stuff and any objects we need to delete
// - When you make new DX resources, you need to release them here
// - If you don't, you get a lot of scary looking messages in Visual Studio
// --------------------------------------------------------
MyDemoGame::~MyDemoGame()
{
	// Release any D3D stuff that's still hanging out
	//ReleaseMacro(vertexBuffer);
	//ReleaseMacro(indexBuffer);
	// Delete our simple shaders
	delete vertexShader;
	delete pixelShader;
	delete skyVS;
	delete skyPS;
	delete normalMappingPS;
	delete ppVS;
	delete ppPS;
	delete brtPS;
	delete mergePS;
	delete shadowVS;
	delete reflectionShader;

	delete skyBoxMaterial;
	delete _NormalMapMat;
	delete _helixMaterial;
	delete _waterCubeMaterial;
	delete terrainVS;
	delete terrainPS;

	delete myCamera;
	samplerState->Release();
	rasState->Release();
	depthState->Release();
	mRTV->Release();
	bpRTV->Release();
	brtpRTV->Release();
	blendState->Release();
	particleBlendState->Release();
	particleDepthState->Release();

	shadowDSV->Release();
	shadowSRV->Release();
	shadowRS->Release();
	shadowSampler->Release();
	particleTexSRV->Release();

	ImGui_ImplDX11_Shutdown();
	
	delete _skybox;
	delete skyBoxCube;

	delete helixGameObject;
	delete waterCubeGameObject;
	delete fireEmitter;

	std::vector<GameObject*>::iterator it;
	for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
		delete (*it);
	}

	for (it = physicsGameObjects.begin(); it != physicsGameObjects.end(); ++it) {
		delete (*it);
	}

	std::vector<ID3D11ShaderResourceView*>::iterator it1;
	for (it1 = srvContainer.begin(); it1 != srvContainer.end(); ++it1) {
		(*it1)->Release();
	}
	std::vector<Mesh*>::iterator it2;
	for (it2 = meshes.begin(); it2 != meshes.end(); ++it2) {
		delete (*it2);
	}
}

#pragma endregion

#pragma region Initialization

// --------------------------------------------------------
// Initializes the base class (including the window and D3D),
// sets up our geometry and loads the shaders (among other things)
// --------------------------------------------------------
bool MyDemoGame::Init()
{	
	//load physics world
	LoadDynamicsWorld();

	// Camera
	myCamera = new Camera();
	// Call the base class's Init() method to create the window,
	// initialize DirectX, etc.
	if( !DirectXGameCore::Init() )
		return false;

	// Helper methods to create something to draw, load shaders to draw it 
	// with and set up matrices so we can see how to pass data to the GPU.
	//  - For your own projects, feel free to expand/replace these.	
	LoadShaders(); 
	CreateGeometry();
	CreateMatrices();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives we'll be using and how to interpret them
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create Materials
	skyBoxMaterial = new Material(&skyVS, &skyPS);
	skyBoxMaterial->Skybox(&skyVS, &skyPS, &device, &deviceContext, &samplerState, &skySRV, &rasState, &depthState, L"SunnyCubeMap.dds");
																																//they should be put into materials
	_NormalMapMat = new Material(&vertexShader, &normalMappingPS, &device, &deviceContext, &samplerState, &texSRV,L"bricks2.jpg", &nMapSRV, L"bricks2_normal.jpg",&dMapSRV,L"bricks2_disp.jpg");
	// Create Material -> Params (Vertexshader, Pixel shader)

	_helixMaterial = new Material(&vertexShader, &pixelShader, &device, &deviceContext, &samplerState, &helixTexSRV, L"bricks2.jpg"); //if I can find 3 textures of differing qualities
	_floorMat = new Material(&vertexShader, &pixelShader, &device, &deviceContext, &samplerState, &helixTexSRV, L"sphere.jpg");																																								 //they should be put into materials
	
	//_waterCubeMaterial = new Material(&vertexShader, &pixelShader, &device, &deviceContext, &samplerState, &waterTexSRV, L"skyblue.jpg");
	_waterCubeMaterial = new Material(&vertexShader, &reflectionShader, &device, &deviceContext, &samplerState, &waterTexSRV, L"skyblue.jpg", &nMapSRV, L"waternormal1.png");

	terrainMaterial = new Material(&vertexShader, &terrainPS, &device, &deviceContext, &samplerState, &terrainSRV, L"terraintexture.jpg");

	CreateSceneObjects();
	srvContainer.push_back(texSRV);
	srvContainer.push_back(nMapSRV);
	srvContainer.push_back(dMapSRV);
	srvContainer.push_back(skySRV);
	srvContainer.push_back(mSRV);
	srvContainer.push_back(bpSRV);
	srvContainer.push_back(brtSRV);
	srvContainer.push_back(helixTexSRV);
	srvContainer.push_back(waterTexSRV);


	GameObject* cube = new GameObject(_cube, _NormalMapMat);
	gameObjects.push_back(cube);
	GameObject* cube2 = new GameObject(_cube2, _NormalMapMat);
	gameObjects.push_back(cube2);

	GameObject* terrainGameObject = new GameObject(_terrain, terrainMaterial);
	gameObjects.push_back(terrainGameObject);

	GameObject* cube3 = new GameObject(_cube, _floorMat);
	gameObjects.push_back(cube3);
	cube3->SetScale(XMFLOAT3(200, 1, 200));
	cube3->SetYPosition(-5);

	/*GameObject* building = new GameObject(_cube, _NormalMapMat);
	gameObjects.push_back(building);
	building->SetScale(XMFLOAT3(10, 20, 10));
	building->SetYPosition(3);
	building->SetZPosition(30);
	building->SetRotation(XMFLOAT3(0, 0, 1.57f));
*/
	//water object
	waterCubeGameObject = new GameObject(_waterCube, _waterCubeMaterial);

	//blending object
	helixGameObject = new GameObject(_helix, _helixMaterial);


	cube->SetXPosition(-2);
	helixGameObject->SetXPosition(2);
	waterCubeGameObject->SetPosition(XMFLOAT3(0.0f, -6.99f, 20.0f));
	waterCubeGameObject->SetScale(XMFLOAT3(5.0f,5.0f,5.0f));

	terrainGameObject->SetPosition(XMFLOAT3(5.0, -4.6, 0.0f));
	skyBoxCube = new GameObject(sbCube, skyBoxMaterial);
	_skybox = new SkyBox(skyBoxCube);
	
	
	//Initialize physics game objects
	// for a physics demo
	for (int i = 0; i < 10;i++)
	{
		GameObject* g = new GameObject(_cube2, _NormalMapMat);
		g->SetDefaultMass();
		g->InitializeRigidBody();
		g->body->activate(true);
		g->SetPosition(XMFLOAT3(0.1+i*4, 0.1+i*4, 0.1+i*4));
		physicsGameObjects.push_back(g);
		physics->dynamicsWorld->addRigidBody(g->body);
		physics->dynamicsWorld->addCollisionObject(g->mPlayerObject);
	}

	//  Initialize Lights

	//Directional Light 
	directionLight.AmbientColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0);
	directionLight.DiffuseColor = XMFLOAT4(0, 0, 0, 0);
	directionLight.Direction = XMFLOAT3(0, 0, -1);

	pixelShader->SetData("directionLight", &directionLight, sizeof(directionLight));
	normalMappingPS->SetData("directionLight", &directionLight, sizeof(directionLight));
	reflectionShader->SetData("directionLight", &directionLight, sizeof(directionLight));
	terrainPS->SetData("directionLight", &directionLight, sizeof(directionLight));
	
	//Point Light
	pointLight.PointLightColor = XMFLOAT4(0, 1, 0, 0);
	pointLight.Position = XMFLOAT3(-.5, 0, -3);
	pointLight.Strength = 1.0f;
	pixelShader->SetData("pointLight", &pointLight, sizeof(pointLight));
	normalMappingPS->SetData("pointLight", &pointLight, sizeof(pointLight));
	reflectionShader->SetData("pointLight", &pointLight, sizeof(pointLight));
	terrainPS->SetData("pointLight", &directionLight, sizeof(directionLight));


	// Specular Light
	specularLight.SpecularStrength = 1;
	specularLight.SpecularColor = XMFLOAT4(0, 0, 0, 1);
	pixelShader->SetData("specularLight", &specularLight, sizeof(specularLight));
	normalMappingPS->SetData("specularLight", &specularLight, sizeof(specularLight));
	reflectionShader->SetData("specularLight", &specularLight, sizeof(specularLight));


	
	// Create a description of the blend state I want
	D3D11_BLEND_DESC blendDesc = {};

	// Set up some of the basic options
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	// Set up the blend options for the first render target
	blendDesc.RenderTarget[0].BlendEnable = true;

	// Settings for how colors (RGB) are blended (ALPHA BLENDING)
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	// Settings for ADDITIVE BLENDING
	//blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	//blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	//blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	// Settings for how the alpha channel is blended
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	// Write masks
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// Create the blend state object
	device->CreateBlendState(&blendDesc, &blendState);

	//Gui initialization
	ImGui_ImplDX11_Init(hMainWnd, device, deviceContext);

	return true;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// - These simple shaders provide helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void MyDemoGame::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, deviceContext);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, deviceContext);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	normalMappingPS = new SimplePixelShader(device, deviceContext);
	normalMappingPS->LoadShaderFile(L"NormalMapping.cso");

	reflectionShader = new SimplePixelShader(device, deviceContext);
	reflectionShader->LoadShaderFile(L"ReflectionShader.cso");


	skyVS = new SimpleVertexShader(device, deviceContext);
	skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, deviceContext);
	skyPS->LoadShaderFile(L"SkyPS.cso");

	terrainVS = new SimpleVertexShader(device, deviceContext);
	terrainVS->LoadShaderFile(L"TerrainVS.cso");

	terrainPS = new SimplePixelShader(device, deviceContext);
	terrainPS->LoadShaderFile(L"TerrainPS.cso");

	ppVS = new SimpleVertexShader(device, deviceContext);
	ppVS->LoadShaderFile(L"BlurVS.cso");

	ppPS = new SimplePixelShader(device, deviceContext);
	ppPS->LoadShaderFile(L"BlurPS.cso");

	mergePS = new SimplePixelShader(device, deviceContext);
	mergePS->LoadShaderFile(L"MergeShader.cso");

	brtPS = new SimplePixelShader(device, deviceContext);
	brtPS->LoadShaderFile(L"Brightness.cso");

	shadowVS = new SimpleVertexShader(device, deviceContext);
	shadowVS->LoadShaderFile(L"ShadowVS.cso");

	particleVS = new SimpleVertexShader(device, deviceContext);
	particleVS->LoadShaderFile(L"ParticleVS.cso");

	particlePS = new SimplePixelShader(device, deviceContext);
	particlePS->LoadShaderFile(L"ParticlePS.cso");

	DirectX::CreateWICTextureFromFile(device, deviceContext, L"particle.jpg", 0, &particleTexSRV);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &samplerState);

	// Create the rasterizer state for the sky
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &rasState);

	// Create the depth stencil state for the sky
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &depthState);

	// Create all the shadow DX stuff ---------------------------
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	shadowMapDesc.Width = shadowMapSize;
	shadowMapDesc.Height = shadowMapSize;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.MiscFlags = 0;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowMapDesc, 0, &shadowTexture);

	// Create the depth/stencil view for the shadow map
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // Gotta give it the D
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture, &dsvDesc, &shadowDSV);

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture, &srvDesc, &shadowSRV);

	// Done with this texture ref
	shadowTexture->Release();

	// Create a better sampler specifically for the shadow map
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.BorderColor[0] = 1.0f;
	sampDesc.BorderColor[1] = 1.0f;
	sampDesc.BorderColor[2] = 1.0f;
	sampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&sampDesc, &shadowSampler);

	// Create a rasterizer for the shadow creation stage (to apply a bias for us)
	D3D11_RASTERIZER_DESC shRastDesc = {};
	shRastDesc.FillMode = D3D11_FILL_SOLID;
	shRastDesc.CullMode = D3D11_CULL_BACK;
	shRastDesc.FrontCounterClockwise = false;
	shRastDesc.DepthClipEnable = true;
	shRastDesc.DepthBias = 1000; // Not world units - this gets multiplied by the "smallest possible value > 0 in depth buffer"
	shRastDesc.DepthBiasClamp = 0.0f;
	shRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shRastDesc, &shadowRS);


	//Particle depth state and blend state
	// A depth state for the particles
	dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepthState);


	// Blend for particles
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	/// Set up particles
	fireEmitter = new Emitter(
		1000,							// Max particles
		100,							// Particles per second
		2,								// Particle lifetime
		1.0f,							// Start size
		0.3f,							// End size
		XMFLOAT4(0, 1, 0.1f, 0.2f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(-0.1, 0.1, 0),				// Start velocity
		XMFLOAT3(0, 1, 0),				// Start position
		XMFLOAT3(0, 1, 0),				// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexSRV);

	explosiveEmitter = new Emitter(
		1000,							// Max particles
		100,							// Particles per second
		2,								// Particle lifetime
		1.0f,							// Start size
		0.3f,							// End size
		XMFLOAT4(1, 0.1f, 0.1f, 0.2f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(-0.1, 0.1, 0),				// Start velocity
		XMFLOAT3(2, 0, 3),				// Start position
		XMFLOAT3(0, 1, 0),				// Start acceleration
		device,
		particleVS,
		particlePS,
		particleTexSRV);


	//main Tex render target
	MakePostProcessContent(mtDesc, rtvDesc, msrvDesc, mTexture, mRTV, mSRV);

	//Get blur render Target ready
	MakePostProcessContent(blurtDesc, blurrtvDesc, blursrvDesc, bpTexture, bpRTV, bpSRV);
	
	//Brightness render target
	MakePostProcessContent(brttDesc, brtvDesc, brtsrvDesc, brtTexture, brtpRTV, brtSRV);
	
}

void MyDemoGame::MakePostProcessContent(D3D11_TEXTURE2D_DESC& tDesc, D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, ID3D11Texture2D*& ppTexture, ID3D11RenderTargetView*& ppRTV, ID3D11ShaderResourceView*& ppSRV) 
{

	tDesc = {};
	tDesc.Height = windowHeight;
	tDesc.Width = windowWidth;
	tDesc.ArraySize = 1;
	tDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tDesc.CPUAccessFlags = 0;
	tDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tDesc.MipLevels = 1;
	tDesc.MiscFlags = 0;
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 0;
	tDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&tDesc, 0, &ppTexture);

	rtvDesc = {};
	rtvDesc.Format = tDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(ppTexture, &rtvDesc, &ppRTV);

	srvDesc = {};
	srvDesc.Format = tDesc.Format;
	srvDesc.Texture2D.MipLevels = tDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(ppTexture, &srvDesc, &ppSRV);

	// Release one reference to the texture
	 ppTexture->Release();

}


void MyDemoGame::CreateGeometry()
{
	_cube = new Mesh(device, "cube.obj");
	meshes.push_back(_cube);
	_cube2 = new Mesh(device, "cube.obj");
	meshes.push_back(_cube2);
	_helix = new Mesh(device, "helix.obj");
	meshes.push_back(_helix);
	sbCube = new Mesh(device, "cube.obj");
	meshes.push_back(sbCube);
	_waterCube = new Mesh(device, "cube.obj");
	meshes.push_back(_waterCube);

	_terrain = new Mesh(device, "../DirectX11_Starter/data/heightmap01.bmp",true);
	meshes.push_back(_terrain);
}


// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void MyDemoGame::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//   update when/if the object moves (every frame)
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!
    
	// Create the View matrix
	// - In an actual game, recreate this matrix when the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction you want it to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up  = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V   = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	XMMATRIX shView = XMMatrixLookToLH(
		XMVectorSet(0, 20, -20, 0),
		XMVectorSet(0, -1, 1, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowView, XMMatrixTranspose(shView));

	XMMATRIX shProj = XMMatrixOrthographicLH(
		1000.0f,		// Width in world units
		1000.0f,		// Height in world units
		0.1f,		// Near plane distance
		10000.0f);	// Far plane distance

	XMStoreFloat4x4(&shadowProj, XMMatrixTranspose(shProj));
	myCamera->OnResize(aspectRatio);
}

#pragma endregion

#pragma region Window Resizing

// --------------------------------------------------------
// Handles resizing DirectX "stuff" to match the (usually) new
// window size and updating our projection matrix to match
// --------------------------------------------------------
void MyDemoGame::OnResize()
{
	// Handle base-level DX resize stuff
	DirectXGameCore::OnResize();
	myCamera->OnResize(aspectRatio);
}
#pragma endregion

#pragma region Game Loop

// --------------------------------------------------------
// Update your game here - take input, move objects, etc.
// --------------------------------------------------------
float x = 0;
void MyDemoGame::UpdateScene(float deltaTime, float totalTime)
{
	GetCursorPos(&p);

	//update physics
	UpdatePhysicsWorld(static_cast<btScalar>(totalTime));

	fireEmitter->Update(deltaTime);
	explosiveEmitter->Update(deltaTime);
	if (btnState & 0x0001) {
		OnMouseDown(btnState, p.x, p.y);

	}
	
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
	
	//Cube->SetXPosition((totalTime));
	if (GetAsyncKeyState(VK_SPACE)) {
		//isBloom = true;
		myCamera->SetRotationY(sinf(totalTime));
		myCamera->VerticalMovement(10.0f*deltaTime);
	}
	if (GetAsyncKeyState('X') & 0x8000) {
		myCamera->VerticalMovement(-10.0f*deltaTime);
		//isBloom = false;
	}
	if (GetAsyncKeyState('W') & 0x8000) {
		myCamera->Forward(10.0f*deltaTime);
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		myCamera->Forward(-10.0f*deltaTime);
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		myCamera->Strafe(10.0f*deltaTime);
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		myCamera->Strafe(-10.0f*deltaTime);
	}
	if (GetAsyncKeyState('K') & 0x8000) {
		physicsGameObjects.at(0)->body->applyCentralImpulse(btVector3(1.0f, 0.0f, 0.f));
	}

	if (GetAsyncKeyState('J') & 0x8000) {
		physicsGameObjects.at(0)->body->applyCentralImpulse(btVector3(-1.0f, 0.0f, 0.f));
	}

	if (GetAsyncKeyState('I') & 0x8000) {
		physicsGameObjects.at(0)->body->applyCentralImpulse(btVector3(0.0f, 0.0f, 1.0f));
	}

	if (GetAsyncKeyState('M') & 0x8000) {
		physicsGameObjects.at(0)->body->applyCentralImpulse(btVector3(0.0f, 0.0f, -1.0f));
	}
	
	std::vector<GameObject*>::iterator it;
	int temp = 0;
	for (it = gameObjects.begin(); it != gameObjects.end(); ++it)
	{
		GameObject* ge = gameObjects.at(temp);
		if (ge->GetTag() == 't') {
			(*it)->SetRotation(XMFLOAT3(totalTime, totalTime, totalTime));
		}
		temp++;
	}

	myCamera->Update();
	viewMatrix = myCamera->GetviewMatrix();

	//waterCubeGameObject->SetRotationY(totalTime*0.5f);


}

void MyDemoGame::DrawShadows() {

	std::vector<GameObject*>::iterator it;
	int temp = 0;
	for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
		GameObject* ge = gameObjects.at(temp);
		vertexShader->SetMatrix4x4("world", ge->GetWorldMatrix());
		vertexShader->SetMatrix4x4("view", myCamera->GetviewMatrix());
		vertexShader->SetMatrix4x4("projection", myCamera->GetProjectionMatrix());
		vertexShader->SetMatrix4x4("shadowView", shadowView);
		vertexShader->SetMatrix4x4("shadowProjection", shadowProj);
		normalMappingPS->SetShaderResourceView("shadowMap", shadowSRV);
		normalMappingPS->SetSamplerState("shadowSampler", shadowSampler);

		(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		(*it)->Draw(deviceContext);
		temp++;
	}



	pixelShader->SetShaderResourceView("shadowMap", 0);



}
// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void MyDemoGame::DrawScene(float deltaTime, float totalTime)
{
	//Imgui code
	ImGui_ImplDX11_NewFrame();

	ImGui::Begin("Fancy GGP Game Engine");
	ImGui::Text("GUI Frame Work");
	if (ImGui::Button("Toggle Bloom"))
		isBloom ^= 1;
	if (ImGui::Button("Toggle Blend"))
		isBlend ^= 1;
	if (ImGui::Button("Fire Particles"))
		isParticle ^= 1;
	ImGui::End();

	if (isBloom)
	{
		// Set buffers in the input assembler
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
	
		// Background color (Cornflower Blue in this case) for clearing
		const float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		RenderShadowMap();

		//Swap to new Render Target to draw there !
		deviceContext->OMSetRenderTargets(1, &mRTV, depthStencilView);
		deviceContext->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		float factors[4] = { 1,1,1,1 };
		if(isBlend)
		{
			// Turn Off the blend state
			
			deviceContext->OMSetBlendState(
				NULL,
				factors,
				0xFFFFFFFF);
		}

		// Clear the render target and depth buffer (erases what's on the screen)
		//  - Do this ONCE PER FRAME
		//  - At the beginning of DrawScene (before drawing *anything*)
		deviceContext->ClearRenderTargetView(mRTV, color);

		/*std::vector<GameObject*>::iterator it;
		for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
		}*/
		std::vector<GameObject*>::iterator it;
		for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
			//GameObject* ge = gameObjects.at(temp);
			vertexShader->SetMatrix4x4("world", (*it)->GetWorldMatrix());
			vertexShader->SetMatrix4x4("view", myCamera->GetviewMatrix());
			vertexShader->SetMatrix4x4("projection", myCamera->GetProjectionMatrix());
			vertexShader->SetMatrix4x4("shadowView", shadowView);
			vertexShader->SetMatrix4x4("shadowProjection", shadowProj);
			normalMappingPS->SetShaderResourceView("shadowMap", shadowSRV);
			normalMappingPS->SetSamplerState("shadowSampler", shadowSampler);

			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
		}

		for (it = physicsGameObjects.begin(); it != physicsGameObjects.end(); ++it) 
		{
			vertexShader->SetMatrix4x4("world", (*it)->GetWorldMatrix());
			vertexShader->SetMatrix4x4("view", myCamera->GetviewMatrix());
			vertexShader->SetMatrix4x4("projection", myCamera->GetProjectionMatrix());
			vertexShader->SetMatrix4x4("shadowView", shadowView);
			vertexShader->SetMatrix4x4("shadowProjection", shadowProj);
			normalMappingPS->SetShaderResourceView("shadowMap", shadowSRV);
			normalMappingPS->SetSamplerState("shadowSampler", shadowSampler);

			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
		}



		pixelShader->SetShaderResourceView("shadowMap", 0);

		if (isBlend)
		{
			// Turn on the blend state
			deviceContext->OMSetBlendState(
				blendState,
				factors,
				0xFFFFFFFF);
		}

		helixGameObject->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		helixGameObject->Draw(deviceContext);

		if (isBlend)
		{
			// Turn off the blend state
			deviceContext->OMSetBlendState(
				NULL,
				factors,
				0xFFFFFFFF);
		}

		reflectionShader->SetShaderResourceView("skyTexture", skySRV);
		waterCubeGameObject->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		waterCubeGameObject->Draw(deviceContext);

		_skybox->skyBox->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		_skybox->Draw(deviceContext);

		ImGui::Render();

		deviceContext->RSSetState(0);
		deviceContext->OMSetDepthStencilState(0, 0);

	//----------------------------------------------------------EXTRACT BRIGHTNESS----------------------------------------//
	
	
			// Swap to the back buffer and do post processing
			deviceContext->OMSetRenderTargets(1, &brtpRTV, depthStencilView);
			deviceContext->ClearDepthStencilView(
				depthStencilView,
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				1.0f,
				0);
			deviceContext->ClearRenderTargetView(brtpRTV, color);

			// Set up post processing shaders
			ppVS->SetShader(true);

			brtPS->SetShaderResourceView("pixels", mSRV);
			brtPS->SetSamplerState("trilinear", samplerState);
			brtPS->SetShader(true);

			ID3D11Buffer* nothing = 0;
			deviceContext->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
			deviceContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	
			// Actually draw the triangle that covers the screen
			deviceContext->Draw(3, 0);



			// Unbind the render target SRV
			brtPS->SetShaderResourceView("pixels", 0);
			ppVS->SetShader(0);
			brtPS->SetShader(0);

			//---------------------------------------------BLUR------------------------------------------------------------------//
				// Swap to the back buffer and do post processing
			deviceContext->OMSetRenderTargets(1, &bpRTV, depthStencilView);
			deviceContext->ClearDepthStencilView(
				depthStencilView,
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				1.0f,
				0);
			deviceContext->ClearRenderTargetView(bpRTV, color);

			// Set up post processing shaders
			ppVS->SetShader(true);

			ppPS->SetInt("blurAmount", 6);
			ppPS->SetFloat("pixelWidth", 1.0f / windowWidth);
			ppPS->SetFloat("pixelHeight", 1.0f / windowHeight);
			ppPS->SetShaderResourceView("pixels", brtSRV);
			ppPS->SetSamplerState("trilinear", samplerState);
			ppPS->SetShader(true);

			ID3D11Buffer* nothing1 = 0;
			deviceContext->IASetVertexBuffers(0, 1, &nothing1, &stride, &offset);
			deviceContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

			// Actually draw the triangle that covers the screen
			deviceContext->Draw(3, 0);

	
			// Unbind the render target SRV
			ppPS->SetShaderResourceView("pixels", 0);
			ppVS->SetShader(0);
			ppPS->SetShader(0);
			//-----------------------------------------------FINAL DISPLAY ON SCREEN--------------------------------------------//
				// Swap to the back buffer and do post processing
			deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
			deviceContext->ClearRenderTargetView(renderTargetView, color);
			deviceContext->ClearDepthStencilView(
				depthStencilView,
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				1.0f,
				0);

			// Set up post processing shaders
			ppVS->SetShader(true);
			mergePS->SetShaderResourceView("pixels", bpSRV);
			mergePS->SetShaderResourceView("mainTex", mSRV);  //brtSRV, mSRV, bpSRV
			mergePS->SetSamplerState("trilinear", samplerState);
			mergePS->SetShader(true);

			ID3D11Buffer* nothing2 = 0;
			deviceContext->IASetVertexBuffers(0, 1, &nothing2, &stride, &offset);
			deviceContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

			
			//ImGui::Render();
			
			deviceContext->Draw(3, 0);
	
		

			// Unbind the render target SRV
			mergePS->SetShaderResourceView("pixels", 0);
			mergePS->SetShaderResourceView("mainTex", 0);
			ppVS->SetShader(0);
			mergePS->SetShader(0);
			// Present the buffer
			//  - Puts the image we're drawing into the window so the user can see it
			//  - Do this exactly ONCE PER FRAME
			
			


	}

	else
	{

		// Background color (Cornflower Blue in this case) for clearing
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		RenderShadowMap();
		deviceContext->ClearRenderTargetView(renderTargetView, color);
		deviceContext->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		// Turn Off the blend state
		float factors[4] = { 1,1,1,1 };
		if (isBlend)
		{
			deviceContext->OMSetBlendState(
				NULL,
				factors,
				0xFFFFFFFF);
		}


		pixelShader->SetFloat3("cameraPosition", myCamera->camPosition);
		normalMappingPS->SetFloat3("cameraPosition", myCamera->camPosition);

		/*std::vector<GameObject*>::iterator it;
		for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
		}*/
		std::vector<GameObject*>::iterator it;
		int temp = 0;
		for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
			GameObject* ge = gameObjects.at(temp);
			vertexShader->SetMatrix4x4("world", ge->GetWorldMatrix());
			vertexShader->SetMatrix4x4("view", myCamera->GetviewMatrix());
			vertexShader->SetMatrix4x4("projection", myCamera->GetProjectionMatrix());
			vertexShader->SetMatrix4x4("shadowView", shadowView);
			vertexShader->SetMatrix4x4("shadowProjection", shadowProj);
			normalMappingPS->SetShaderResourceView("shadowMap", shadowSRV);
			normalMappingPS->SetSamplerState("shadowSampler", shadowSampler);

			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
			temp++;
		}

		for (it = physicsGameObjects.begin(); it != physicsGameObjects.end(); ++it)
		{
			vertexShader->SetMatrix4x4("world", (*it)->GetWorldMatrix());
			vertexShader->SetMatrix4x4("view", myCamera->GetviewMatrix());
			vertexShader->SetMatrix4x4("projection", myCamera->GetProjectionMatrix());
			vertexShader->SetMatrix4x4("shadowView", shadowView);
			vertexShader->SetMatrix4x4("shadowProjection", shadowProj);
			normalMappingPS->SetShaderResourceView("shadowMap", shadowSRV);
			normalMappingPS->SetSamplerState("shadowSampler", shadowSampler);

			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
		}

		pixelShader->SetShaderResourceView("shadowMap", 0);

		if (isBlend)
		{
			deviceContext->OMSetBlendState(
				blendState,
				factors,
				0xFFFFFFFF);
		}

		helixGameObject->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		helixGameObject->Draw(deviceContext);

		if(isBlend)
		{
			deviceContext->OMSetBlendState(
				NULL,
				factors,
				0xFFFFFFFF);
		}

		reflectionShader->SetShaderResourceView("skyTexture", skySRV);
		waterCubeGameObject->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		waterCubeGameObject->Draw(deviceContext);

		_skybox->skyBox->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		_skybox->Draw(deviceContext);

		if (isParticle)
		{
			
			// Reset rasterizer state
			deviceContext->RSSetState(0);

			deviceContext->OMSetBlendState(
				blendState,
				factors,
				0xFFFFFFFF);



			// Particle states
			float blend[4] = { 1,1,1,1 };
			deviceContext->OMSetBlendState(particleBlendState, blend, 0xffffffff);  // Additive blending
			deviceContext->OMSetDepthStencilState(particleDepthState, 0);			// No depth WRITING

																					// Draw the emitter
			fireEmitter->Draw(deviceContext, myCamera);
			explosiveEmitter->Draw(deviceContext, myCamera);

			deviceContext->OMSetBlendState(
				NULL,
				factors,
				0xFFFFFFFF);
		}



		ImGui::Render();
		deviceContext->RSSetState(0);
		deviceContext->OMSetDepthStencilState(0, 0);
		
	}

	
	
	HR(swapChain->Present(0, 0));
}

#pragma endregion

void MyDemoGame::RenderShadowMap()
{
	// Initial setup
	deviceContext->OMSetRenderTargets(0, 0, shadowDSV);
	deviceContext->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	deviceContext->RSSetState(shadowRS);

	// We need a viewport!  This defines how much of the render target to render into
	D3D11_VIEWPORT shadowVP = viewport;
	shadowVP.Width = (float)shadowMapSize;
	shadowVP.Height = (float)shadowMapSize;
	deviceContext->RSSetViewports(1, &shadowVP);

	// Turn on the correct shaders
	shadowVS->SetShader(false); // Don't copy any data yet
	shadowVS->SetMatrix4x4("view", shadowView);
	shadowVS->SetMatrix4x4("projection", shadowProj);
	deviceContext->PSSetShader(0, 0, 0); // Turn off the pixel shader

										 // Actually render everything
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	std::vector<GameObject*>::iterator it;
	int temp = 0;
	for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {



		GameObject* ge = gameObjects.at(temp);
		ID3D11Buffer* vb = ge->getMesh()->GetVertexBuffer();
		ID3D11Buffer* ib = ge->getMesh()->GetIndexBuffer();

		// Set buffers in the input assembler
		deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		deviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		shadowVS->SetMatrix4x4("world", ge->GetWorldMatrix());

		// Actually copy the data for this object
		shadowVS->CopyAllBufferData();

		// Finally do the actual drawing
		deviceContext->DrawIndexed(ge->getMesh()->GetIndexCount(), 0, 0);
		temp++;
		
	}
	//render shadow map for physics objects
	for (it = physicsGameObjects.begin(); it != physicsGameObjects.end(); ++it) {

		ID3D11Buffer* vb = (*it)->getMesh()->GetVertexBuffer();
		ID3D11Buffer* ib = (*it)->getMesh()->GetIndexBuffer();

		// Set buffers in the input assembler
		deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		deviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		shadowVS->SetMatrix4x4("world", (*it)->GetWorldMatrix());

		// Actually copy the data for this object
		shadowVS->CopyAllBufferData();

		// Finally do the actual drawing
		deviceContext->DrawIndexed((*it)->getMesh()->GetIndexCount(), 0, 0);
	}

	// Revert to original DX state
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	deviceContext->RSSetViewports(1, &viewport);
	deviceContext->RSSetState(0);
}

void MyDemoGame::CreateSceneObjects()
{
	//Create all the Game or Test geometry
	//float x = 0, y = 0, z = 0; //x is across, y is up, z is back
	for (float x= 0; x < 5; x++) {
		for (float y = 0; y < 5; y++) {
			for (float z = 0; z < 5; z++) {

				GameObject* building = new GameObject(_cube, _NormalMapMat);
				gameObjects.push_back(building);
				building->SetPosition(XMFLOAT3(x*x, y*y, z*z));
				building->SetTag('t');
			}
		}
	}
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
//
// Feel free to add code to this method
// --------------------------------------------------------
void MyDemoGame::OnMouseDown(WPARAM btnState, int x, int y)
{
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hMainWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
//
// Feel free to add code to this method
// --------------------------------------------------------
void MyDemoGame::OnMouseUp(WPARAM btnState, int x, int y)
{
	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
//
// Feel free to add code to this method
// --------------------------------------------------------
void MyDemoGame::OnMouseMove(WPARAM btnState, int x, int y)
{
	// Save the previous mouse position, so we have it for the future
	//hold left button and move camera
	if (btnState & 0x0001)
	{
		int diffX = x - prevMousePos.x;
		int diffY = y - prevMousePos.y;
		if (diffX > 1000 || diffX <-1000 || diffY > 1000 || diffY< -1000) {
			diffX = diffY = 0;
		}
		myCamera->MouseMovement(diffY * 0.001f, diffX* 0.001f);
		prevMousePos.x = x;
		prevMousePos.y = y;
	}
}

//Update physics world in regular intervals
void MyDemoGame::UpdatePhysicsWorld(float elapsedTime)
{
	// fixed 1/60 timestep
	physics->dynamicsWorld->stepSimulation(1 / 10.0f, 1);

	XMFLOAT3 mat;
	const btCollisionObjectArray& objectArray = physics->dynamicsWorld->getCollisionObjectArray();

	for (int i = 0; i < physicsGameObjects.size(); i++)
	{
		physicsGameObjects.at(i)->body->activate(true);

		btRigidBody* pBody = physicsGameObjects.at(i)->body;
		if (pBody && pBody->getMotionState())
		{
			btTransform trans = physicsGameObjects.at(i)->startTransform;
			pBody->getMotionState()->getWorldTransform(trans);
			XMFLOAT3 pos = XMFLOAT3(trans.getOrigin());
			physicsGameObjects.at(i)->SetPosition(pos);
			physicsGameObjects.at(i)->SetRotation(trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ());
			physicsGameObjects.at(i)->SetWorldMatrix();
		}
	}
}
#pragma endregion