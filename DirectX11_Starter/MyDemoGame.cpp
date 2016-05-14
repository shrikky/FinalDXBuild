// ----------------------------------------------------------------------------
//  A few notes on project settings
//
//  - The project is set to use the UNICODE character set
//    - This was changed in Project Properties > Config Properties > General > Character Set
//    - This basically adds a "#define UNICODE" to the project
//
//  - The include directories were automagically correct, since the DirectX 
//    headers and libs are part of the windows SDK
//    - For instance, $(WindowsSDK_IncludePath) is set as a project include 
//      path by default.  That's where the DirectX headers are located.
//
//  - Two libraries had to be manually added to the Linker Input Dependencies
//    - d3d11.lib
//    - d3dcompiler.lib
//    - This was changed in Project Properties > Config Properties > Linker > Input > Additional Dependencies
//
//  - The Working Directory was changed to match the actual .exe's 
//    output directory, since we need to load the compiled shader files at run time
//    - This was changed in Project Properties > Config Properties > Debugging > Working Directory
//
// ----------------------------------------------------------------------------

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

	delete skyBoxMaterial;
	delete _NormalMapMat;
	delete _helixMaterial;

	delete myCamera;
	samplerState->Release();
	rasState->Release();
	depthState->Release();
	mRTV->Release();
	bpRTV->Release();
	brtpRTV->Release();
	blendState->Release();

	ImGui_ImplDX11_Shutdown();
	
	delete _skybox;
	delete skyBoxCube;

	delete helixGameObject;

	std::vector<GameObject*>::iterator it;
	for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
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
{	// Camera

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
																																									 //they should be put into materials
	
	srvContainer.push_back(texSRV);
	srvContainer.push_back(nMapSRV);
	srvContainer.push_back(dMapSRV);
	srvContainer.push_back(skySRV);
	srvContainer.push_back(mSRV);
	srvContainer.push_back(bpSRV);
	srvContainer.push_back(brtSRV);
	srvContainer.push_back(helixTexSRV);


	GameObject* cube = new GameObject(_cube, _NormalMapMat);
	gameObjects.push_back(cube);
	GameObject* cube2 = new GameObject(_cube2, _NormalMapMat);
	gameObjects.push_back(cube2);

	//blending object
	helixGameObject = new GameObject(_helix, _helixMaterial);

	cube->SetXPosition(-2);
	helixGameObject->SetXPosition(2);


	skyBoxCube = new GameObject(sbCube, skyBoxMaterial);
	_skybox = new SkyBox(skyBoxCube);
	//  Initialize Lights

	//Directional Light 
	directionLight.AmbientColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0);
	directionLight.DiffuseColor = XMFLOAT4(0, 0, 0, 0);
	directionLight.Direction = XMFLOAT3(0, 0, -1);

	pixelShader->SetData("directionLight", &directionLight, sizeof(directionLight));
	normalMappingPS->SetData("directionLight", &directionLight, sizeof(directionLight));

	
	//Point Light
	pointLight.PointLightColor = XMFLOAT4(0, 1, 0, 0);
	pointLight.Position = XMFLOAT3(-.5, 0, -3);
	pointLight.Strength = 1.0f;
	pixelShader->SetData("pointLight", &pointLight, sizeof(pointLight));
	normalMappingPS->SetData("pointLight", &pointLight, sizeof(pointLight));


	// Specular Light
	specularLight.SpecularStrength = 1;
	specularLight.SpecularColor = XMFLOAT4(0, 0, 0, 1);
	pixelShader->SetData("specularLight", &specularLight, sizeof(specularLight));
	normalMappingPS->SetData("specularLight", &specularLight, sizeof(specularLight));


	
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
	/*ImGui_ImplDX11_Init(hMainWnd, device, deviceContext);*/

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


	skyVS = new SimpleVertexShader(device, deviceContext);
	skyVS->LoadShaderFile(L"SkyVS.cso");

	skyPS = new SimplePixelShader(device, deviceContext);
	skyPS->LoadShaderFile(L"SkyPS.cso");

	ppVS = new SimpleVertexShader(device, deviceContext);
	ppVS->LoadShaderFile(L"BlurVS.cso");

	ppPS = new SimplePixelShader(device, deviceContext);
	ppPS->LoadShaderFile(L"BlurPS.cso");

	mergePS = new SimplePixelShader(device, deviceContext);
	mergePS->LoadShaderFile(L"MergeShader.cso");

	brtPS = new SimplePixelShader(device, deviceContext);
	brtPS->LoadShaderFile(L"Brightness.cso");



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
	if (btnState & 0x0001) {
		OnMouseDown(btnState, p.x, p.y);

	}
	
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
	
	//Cube->SetXPosition((totalTime));
	if (GetAsyncKeyState(VK_SPACE)) {
		isBloom = true;
		myCamera->SetRotationY(sinf(totalTime));
		myCamera->VerticalMovement(10.0f*deltaTime);
	}
	if (GetAsyncKeyState('X') & 0x8000) {
		myCamera->VerticalMovement(-10.0f*deltaTime);
		isBloom = false;
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
	
	//std::vector<GameObject*>::iterator it;
	//for (it = gameObjects.begin(); it != gameObjects.end(); ++it)
	//{
	//	(*it)->SetRotation(XMFLOAT3(totalTime, totalTime, totalTime));
	//}

	myCamera->Update();
	viewMatrix = myCamera->GetviewMatrix();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void MyDemoGame::DrawScene(float deltaTime, float totalTime)
{
	
	if (isBloom)
	{
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	//Swap to new Render Target to draw there !
	deviceContext->OMSetRenderTargets(1, &mRTV, depthStencilView);
	deviceContext->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	// Turn Off the blend state
	float factors[4] = { 0,0,0,1 };
	deviceContext->OMSetBlendState(
		NULL,
		factors,
		0xFFFFFFFF);

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of DrawScene (before drawing *anything*)
	deviceContext->ClearRenderTargetView(mRTV, color);

	// Turn on the blend state
	deviceContext->OMSetBlendState(
		blendState,
		factors,
		0xFFFFFFFF);

		std::vector<GameObject*>::iterator it;
		for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
		}
	helixGameObject->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
	helixGameObject->Draw(deviceContext);
	_skybox->skyBox->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
	_skybox->Draw(deviceContext);

	deviceContext->RSSetState(0);
	deviceContext->OMSetDepthStencilState(0, 0);
	

	// Turn off the blend state
	deviceContext->OMSetBlendState(
		NULL,
		factors,
		0xFFFFFFFF);

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
		deviceContext->ClearRenderTargetView(renderTargetView, color);
		deviceContext->ClearDepthStencilView(
			depthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);
		// Turn Off the blend state

		float factors[4] = { 0,0,0,1 };

		deviceContext->OMSetBlendState(
			blendState,
			factors,
			0xFFFFFFFF);


		pixelShader->SetFloat3("cameraPosition", myCamera->camPosition);
		normalMappingPS->SetFloat3("cameraPosition", myCamera->camPosition);

		std::vector<GameObject*>::iterator it;
		for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
			(*it)->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
			(*it)->Draw(deviceContext);
		}

		helixGameObject->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		helixGameObject->Draw(deviceContext);

		deviceContext->OMSetBlendState(
			NULL,
			factors,
			0xFFFFFFFF);

		_skybox->skyBox->PrepareMaterial(myCamera->GetviewMatrix(), myCamera->GetProjectionMatrix());
		_skybox->Draw(deviceContext);

		deviceContext->RSSetState(0);
		deviceContext->OMSetDepthStencilState(0, 0);


	}
	HR(swapChain->Present(0, 0));
}

#pragma endregion

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
#pragma endregion