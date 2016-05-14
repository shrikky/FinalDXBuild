#pragma once
#include <d3d11.h>
#include "DirectXGameCore.h"
#include "Vertex.h"
#include <fstream>
#include <vector>
using namespace DirectX;
class Mesh
{
public:

	// Buffer var
	ID3D11Buffer* vBuffer;
	ID3D11Buffer* iBuffer;

	//General public Var
	int numOfIndices;

	// Functions
	// Get Vertex Buffer
	ID3D11Buffer* GetVertexBuffer();
	// Get Index Buffer
	ID3D11Buffer* GetIndexBuffer();
	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the filea
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

	// Get number of Indices
	int GetIndexCount();


	Mesh(int vertNum, Vertex* vert, int indNum, unsigned int* indices, ID3D11Device* deviceObj);
	Mesh( ID3D11Device* deviceObj, char* filename);
	void CalculateTangents(Vertex* vert, int vertNum, unsigned int* indices, int indNum);
	void MakePostProcessContent();
	~Mesh();

};

