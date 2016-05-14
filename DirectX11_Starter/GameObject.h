#pragma once

#include <DirectXMath.h>
#include "DirectXGameCore.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include <d3d11.h>
#include "Material.h"
#include "imgui_impl_dx11.h"

using namespace DirectX;
class GameObject
{
public:
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 modelMatrix;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	Mesh* gameObjectMesh;
	Material* gameObjectmaterial;
	XMFLOAT4X4 GetWorldMatrix() {
		return worldMatrix;
	}
	XMFLOAT3 GetPosition() {
		return position;
	}
	XMFLOAT3 GetRotation() {
		return position;
	}
	XMFLOAT3 GetScale() {
		return position;
	}

	float GetXPosition() {
		return position.x;
	}
	void SetXPosition(float posx) {
		position.x = posx;
		SetWorldMatrix();
	}
	void SetYPosition(float posy) {
		position.y = posy;
		SetWorldMatrix();
	}
	void SetZPosition(float posz) {
		position.z = posz;
		SetWorldMatrix();
	}

	void SetPosition(XMFLOAT3 pos) {
		position = pos;
		SetWorldMatrix();
	}
	void SetRotation(XMFLOAT3 rot) {
		rotation = rot;
		SetWorldMatrix();
	}

	void SetRotationY(float y) {
		rotation.y = y;
		SetWorldMatrix();
	}
	void SetScale(XMFLOAT3 _scale) {
		 scale = _scale;
		 SetWorldMatrix();
	}

	void Move();
	void SetWorldMatrix();
	void MoveForward();
	void Draw(ID3D11DeviceContext* deviceContext);
	void PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 proj);
	GameObject(Mesh* mesh, Material* material);
	~GameObject();
private:


};

