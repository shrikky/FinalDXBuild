#include "GameObject.h"



GameObject::GameObject(Mesh *mesh, Material* material)
{
	gameObjectMesh = mesh;
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	gameObjectmaterial = material;
	SetWorldMatrix();
}


GameObject::~GameObject()
{
}

void GameObject::Draw(ID3D11DeviceContext* deviceContext) {

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// Draw the mesh
	ID3D11Buffer* temp = gameObjectMesh->GetVertexBuffer(); // Returns vBuffer which is a pointer to the ID3D11, that it 
	deviceContext->IASetVertexBuffers(0, 1, &temp, &stride, &offset);
	deviceContext->IASetIndexBuffer(gameObjectMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);


	// Finally do the actual drawing
	//  - Do this ONCE PER OBJECT you intend to draw
	//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER

	deviceContext->DrawIndexed(
		gameObjectMesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,								// Offset to the first index we want to use
		0);

	gameObjectmaterial->vertexShader->SetShader(false);
	gameObjectmaterial->pixelShader->SetShader(false);
}
void GameObject::SetWorldMatrix() {
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotY = XMMatrixRotationY(rotation.y);
	XMMATRIX rotX = XMMatrixRotationX(rotation.x);
	XMMATRIX rotZ = XMMatrixRotationZ(rotation.z);
	XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX model = scaleMatrix * rotZ * rotY * rotX * trans;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(model));	//Setting matrix as transpose
}
void GameObject::Move() {
	position.x = 5.0f;
	SetWorldMatrix();
}
void GameObject::PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 proj) {

	gameObjectmaterial->vertexShader->SetMatrix4x4("world", worldMatrix);
	gameObjectmaterial->vertexShader->SetMatrix4x4("model", modelMatrix);
	gameObjectmaterial->vertexShader->SetMatrix4x4("view", view);
	gameObjectmaterial->vertexShader->SetMatrix4x4("projection", proj);
	gameObjectmaterial->UpdateShaderResources();
	gameObjectmaterial->vertexShader->SetShader(true);
	gameObjectmaterial->pixelShader->SetShader(true);
}

void GameObject::MoveForward() {

}