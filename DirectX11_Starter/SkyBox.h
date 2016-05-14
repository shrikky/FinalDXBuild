#pragma once
#include <DirectXMath.h>
#include "DirectXGameCore.h"
#include "SimpleShader.h"
#include "GameObject.h"
class SkyBox
{
public:
	SkyBox(GameObject* skybox);
	~SkyBox();
	GameObject* skyBox;
	void Draw(ID3D11DeviceContext* deviceContext);
};

