#include "Camera.h"
#include <iostream>


Camera::Camera()
{
}


Camera::~Camera()
{
}
void Camera::OnResize(float aspectRatio)
{
	// Handle base-level DX resize stuff

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		aspectRatio,		  	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}
void Camera::Update() {

		XMVECTOR _forward = XMLoadFloat3(&forwardVector);
		XMVECTOR _camPos = XMLoadFloat3(&camPosition);
		XMVECTOR _upAxis = XMLoadFloat3(&upDirection);
		XMMATRIX _view = XMLoadFloat4x4(&viewMatrix);
		XMVECTOR _left = XMVector3Cross(_forward, _upAxis);
		XMStoreFloat3(&leftVector, _left);
  		_view = XMMatrixLookToLH(_camPos, _forward, _upAxis);
		XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(_view));
}

void Camera::Forward(float disp) {
	XMVECTOR camPos = XMLoadFloat3(&camPosition);
	XMVECTOR _forward = XMLoadFloat3(&forwardVector);
	camPos += _forward * disp;
	XMStoreFloat3(&camPosition,camPos);

}

void Camera::Rotate(float rot) {

	XMVECTOR forwardDir = XMLoadFloat3(&forwardVector);
	XMVECTOR camPos = XMLoadFloat3(&camPosition);
	XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(0, rot,0);  // Rotate y by 30
	forwardDir = XMVector3Rotate(forwardDir, rotation);
	XMStoreFloat3(&forwardVector, forwardDir);

}

void Camera::MouseMovement(float x, float y) {

	rotX = x;
	rotY = y;
	/*This code cause a wrong camera rotaion, I leave it here for guys to debug
	XMVECTOR forwardDir = XMLoadFloat3(&forwardVector);
	XMVECTOR camPos = XMLoadFloat3(&camPosition);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(rotX, rotY, 0);  // Rotate y by 30
	forwardDir = XMVector3TransformCoord(forwardDir, rotation);
	forwardDir = XMVector3Normalize(forwardDir);
	XMStoreFloat3(&forwardVector, forwardDir);
	*/

	XMVECTOR cameradir = XMLoadFloat3(&forwardVector);
	XMVECTOR cameraup = XMLoadFloat3(&upDirection);
	XMVECTOR newCameraDir = XMLoadFloat3(&forwardVector);
	XMMATRIX PitchMatrix = XMMatrixRotationAxis(-XMVector3Cross(cameradir, cameraup), rotX);
	XMMATRIX YawMatrix = XMMatrixRotationAxis(cameraup, rotY);
	newCameraDir = XMVector3Transform(newCameraDir, PitchMatrix);
	newCameraDir = XMVector3Transform(newCameraDir, YawMatrix);
	XMStoreFloat3(&forwardVector, newCameraDir);
	//XMVECTOR newUpVector = XMLoadFloat3(&upDirection);
	//newUpVector = XMVector3Transform(newUpVector, YawMatrix);
	//XMStoreFloat3(&upDirection, newUpVector);
}

void Camera::VerticalMovement(float disp) {

	XMVECTOR camPos = XMLoadFloat3(&camPosition);
	XMVECTOR _upVector = XMLoadFloat3(&upDirection);
	camPos += _upVector * disp;
	XMStoreFloat3(&camPosition, camPos);
}
void Camera::Strafe(float disp) {

	XMVECTOR camPos = XMLoadFloat3(&camPosition);
	XMVECTOR _right = -XMLoadFloat3(&leftVector);
	camPos += _right * disp;
	XMStoreFloat3(&leftVector, _right);
	XMStoreFloat3(&camPosition, camPos);

}