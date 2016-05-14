#pragma once
#include "DirectXGameCore.h"
#include <DirectXMath.h>
using namespace DirectX;
struct DirectionLight
{
	XMFLOAT4 AmbientColor;
	XMFLOAT4 DiffuseColor;
	XMFLOAT3 Direction;

};
struct PointLight {
	XMFLOAT4 PointLightColor;
	XMFLOAT3 Position;
	float Strength;
};
struct SpecularLight {
	XMFLOAT4 SpecularColor;
	float SpecularStrength;

};
