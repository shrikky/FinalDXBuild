#include "SkyBox.h"



SkyBox::SkyBox(GameObject* skybox)
{
	skyBox = skybox;
}


SkyBox::~SkyBox()
{
}

void SkyBox::Draw(ID3D11DeviceContext* deviceContext) {

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// Draw the mesh
	ID3D11Buffer* temp = skyBox->gameObjectMesh->GetVertexBuffer(); // Returns vBuffer which is a pointer to the ID3D11, that it 
	deviceContext->IASetVertexBuffers(0, 1, &temp, &stride, &offset);
	deviceContext->IASetIndexBuffer(skyBox->gameObjectMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);


	// Finally do the actual drawing
	//  - Do this ONCE PER OBJECT you intend to draw
	//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER

	deviceContext->RSSetState(skyBox->gameObjectmaterial->_rasState);
	deviceContext->OMSetDepthStencilState(skyBox->gameObjectmaterial->_depthState, 0);

	deviceContext->DrawIndexed(
		skyBox->gameObjectMesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,								// Offset to the first index we want to use
		0);


	//deviceContext->RSSetState(0);
	//deviceContext->OMSetDepthStencilState(0, 0);
}