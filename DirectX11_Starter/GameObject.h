#pragma once

#include <DirectXMath.h>
#include "DirectXGameCore.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include <d3d11.h>
#include "Material.h"
#include "imgui_impl_dx11.h"

//bullet physics
#include "btBulletDynamicsCommon.h"
#include"Physics.h"

using namespace DirectX;
class GameObject
{
public:
	char tag;
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 modelMatrix;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	Mesh* gameObjectMesh;
	Material* gameObjectmaterial;

/*-------------------physics stuff---------------------*/
public:
	//create a dynamic rigidbody
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btCollisionShape* colShape;
	// Create Dynamic Objects
	btTransform  startTransform;
	float mass;
	btVector3 localInertia;
	btDefaultMotionState* myMotionState;
	btRigidBody* body;
	btBoxShape * mPlayerBox;
	btCollisionObject * mPlayerObject;

	void InitializeRigidBody();
	void SetMass(float newmass);
	void SetDefaultMass();
	void CleanupPhysicsObjects();
	void SetRigidBodyShape(float scalex, float scaley, float scalez);
/*----------------------physics end----------------------*/
	XMFLOAT4X4 GetWorldMatrix() {
		return worldMatrix;
	}
	Mesh* getMesh() {
		return gameObjectMesh;


	}
	char GetTag() {

		return tag;
	}
	void SetTag(char t) {
		tag = t;
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

	void SetPosition(XMFLOAT3 pos) 
	{
		position = pos;
		SetWorldMatrix();
	}
	void SetRotation(XMFLOAT3 rot)
	{
		rotation = rot;
		SetWorldMatrix();
	}

	void SetRotation(float x, float y, float z)
	{
		rotation.x = x;
		rotation.y = y;
		rotation.z = z;
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

