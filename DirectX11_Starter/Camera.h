#pragma once
#include <DirectXMath.h>
using namespace DirectX;
class Camera
{
public:

	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT3 camPosition = XMFLOAT3(0, 0, -10);
	XMFLOAT3 forwardVector = XMFLOAT3(0, 0, 1);
	XMFLOAT3 upDirection = XMFLOAT3(0, 1, 0);
	XMFLOAT3 leftVector = XMFLOAT3(0, 0, 0);
	float rotY = 0.0f;
	float rotX = 0.0f;
	float rotZ = 0.0f;
	XMFLOAT4X4 GetWorldMatrix(){
		return worldMatrix;
	}
	XMFLOAT4X4 GetviewMatrix() {
		return viewMatrix;
	}
	XMFLOAT4X4 GetProjectionMatrix() {
		return projectionMatrix;
	}
	void SetRotationX(float x) {
		rotX = x;
	}
	void SetRotationY(float y) {
		rotY = y;
	}
	void SetRotationZ(float z) {
		rotZ = z;
	}
	void Update();
	void Forward(float disp);
	void Rotate(float rotation);
	void Strafe(float disp); 
	void VerticalMovement(float disp);
	void MouseMovement(float x, float y);
	void OnResize(float aspectRatio);
	Camera();
	~Camera();
};

