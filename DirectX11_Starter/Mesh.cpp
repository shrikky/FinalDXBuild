#include "Mesh.h"



Mesh::Mesh(int vertNum, Vertex* vert, int indNum, unsigned int* indices, ID3D11Device* deviceObj)
{
	numOfIndices = indNum;
	CalculateTangents(&verts[0], verts.size(), &indices[0], numOfIndices);
	// Create vertexBuffers
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertNum;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	

	// Load Data to Vertex Buffer
	D3D11_SUBRESOURCE_DATA vertData;
	vertData.pSysMem = vert;

	// Create the buffer with the data
	HR(deviceObj->CreateBuffer(&vbd, &vertData, &vBuffer));

	//Create Index Buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * indNum;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	//Load Data to IndexBuffer
	D3D11_SUBRESOURCE_DATA indData;
	indData.pSysMem = indices;


	// Create Index Buffer with the data.
	HR(deviceObj->CreateBuffer(&ibd, &indData, &iBuffer));
}

Mesh::Mesh(ID3D11Device* deviceObj, char* filename) {
	// File input object
	std::ifstream obj(filename); // <-- Replace filename with your parameter

						
	 // Check for successful open
	if (!obj.is_open())
		return;

	
										 // Still good?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the 9 face indices into an array
			unsigned int i[9];
			sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Add the verts to the vector
			verts.push_back(v1);
			verts.push_back(v2);
			verts.push_back(v3);

			// Add three more indices
			indices.push_back(vertCounter++);
			indices.push_back(vertCounter++);
			indices.push_back(vertCounter++);
		}
	}

	// Close
	obj.close();

	numOfIndices = indices.size();
	CalculateTangents(&verts[0], verts.size(), &indices[0], numOfIndices);
	// Create vertexBuffers
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * verts.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	// Load Data to Vertex Buffer
	D3D11_SUBRESOURCE_DATA vertData;
	vertData.pSysMem = &verts[0];
	// Create the buffer with the data
	HR(deviceObj->CreateBuffer(&vbd, &vertData, &vBuffer));

	//Create Index Buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	//Load Data to IndexBuffer
	D3D11_SUBRESOURCE_DATA indData;
	indData.pSysMem = &indices[0];
	// Create Index Buffer with the data.
	HR(deviceObj->CreateBuffer(&ibd, &indData, &iBuffer));


}


Mesh::Mesh(ID3D11Device* deviceObj, char* heightMapFilename, bool isTerrain) {

	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;

	// Open the height map file in binary.
	error = fopen_s(&filePtr, heightMapFilename, "rb");
	if (error != 0)
	{
		//return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1)
	{
		//return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		//return false;
	}

	// Save the dimensions of the TerrainClass.
	m_TerrainClassWidth = bitmapInfoHeader.biWidth;
	m_TerrainClassHeight = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_TerrainClassWidth * m_TerrainClassHeight * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		//return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);

	if (count != imageSize)
	{
		//return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		//return false;
	}

	//// Create the structure to hold the height map data.
	//m_heightMap = new HeightMapType[m_TerrainClassWidth * m_TerrainClassHeight];
	////memset(m_heightMap, 0, sizeof(HeightMapType)*m_TerrainClassWidth*m_TerrainClassHeight);
	//if (!m_heightMap)
	//{
	//	return false;
	//}

	// Initialize the position in the image data buffer.
	k = 0;
	XMFLOAT3 pos;

	// Read the image data into the height map.
	for (j = 0; j<m_TerrainClassHeight; j++)
	{
		for (i = 0; i<m_TerrainClassWidth; i++)
		{
			height = bitmapImage[k];
			index = (m_TerrainClassHeight * j) + i;
			//reducing the scale of terrain

			pos = XMFLOAT3(((float)(i)) / 5, ((float)(height)) / 100, ((float)(j)) / 5);
			positions.push_back(pos);
			k += 3;
		}
	}

	XMFLOAT2 uv;
	float incrementValue, tuCoordinate, tvCoordinate;
	int incrementCount, tuCount, tvCount;
	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)m_TerrainClassWidth;

	// Calculate how many times to repeat the texture.
	incrementCount = m_TerrainClassWidth / TEXTURE_REPEAT;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	for (j = 0; j<m_TerrainClassHeight; j++)
	{
		for (i = 0; i<m_TerrainClassWidth; i++)
		{

			uv = XMFLOAT2(tuCoordinate, tvCoordinate);

			uvs.push_back(uv);

			// Increment the tu texture coordinate by the increment value and increment the index by one.
			tuCoordinate += incrementValue;
			tuCount++;

			// Check if at the far right end of the texture and if so then start at the beginning again.
			if (tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		// Increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		// Check if at the top of the texture and if so then start at the bottom again.
		if (tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}

	XMFLOAT3 norm;
	int index1, index2, index3, index4;
	XMFLOAT3 vertex1, vertex2, vertex3, vector1, vector2, sum, length;

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j<(m_TerrainClassHeight - 1); j++)
	{
		for (i = 0; i<(m_TerrainClassWidth - 1); i++)
		{
			index1 = (j * m_TerrainClassHeight) + i;
			index2 = (j * m_TerrainClassHeight) + (i + 1);
			index3 = ((j + 1) * m_TerrainClassHeight) + i;

			// Get three vertices from the face.
			vertex1.x = positions.at(index1).x;
			vertex1.y = positions.at(index1).y;
			vertex1.z = positions.at(index1).z;

			vertex2.x = positions.at(index2).x;
			vertex2.y = positions.at(index2).y;
			vertex2.z = positions.at(index2).z;

			vertex3.x = positions.at(index3).x;
			vertex3.y = positions.at(index3).y;
			vertex3.z = positions.at(index3).z;

			// Calculate the two vectors for this face.
			vector1.x = vertex1.x - vertex3.x;
			vector1.y = vertex1.y - vertex3.y;
			vector1.z = vertex1.z - vertex3.z;
			vector2.x = vertex3.x - vertex2.x;
			vector2.y = vertex3.y - vertex2.y;
			vector2.z = vertex3.z - vertex2.z;

			index = (j * (m_TerrainClassHeight - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			norm.x = (vector1.y * vector2.z) - (vector1.z * vector2.y);
			norm.y = (vector1.z * vector2.x) - (vector1.x * vector2.z);
			norm.z = (vector1.x * vector2.y) - (vector1.y * vector2.x);

			// Add to the list of normals
			normals.push_back(norm);

		}
	}

	index1 = index2 = index3 = index4 = index = 0;


	for (j = 0; j<(m_TerrainClassHeight - 3); j++)
	{
		for (i = 0; i<(m_TerrainClassWidth - 3); i++)
		{
			index1 = (m_TerrainClassHeight * j) + i;          // Bottom left.
			index2 = (m_TerrainClassHeight * j) + (i + 1);      // Bottom right.
			index3 = (m_TerrainClassHeight * (j + 1)) + i;      // Upper left.
			index4 = (m_TerrainClassHeight * (j + 1)) + (i + 1);  // Upper right.

			// Upper left.							
			Vertex v1;
			v1.Position = positions.at(index3);
			v1.UV = uvs.at(index3);
			v1.Normal = normals.at(index3);

			v1.UV.y = 1.0f - v1.UV.y;
			verts.push_back(v1);
			indices.push_back(vertCounter++);

			// Upper right.
			Vertex v2;
			v2.Position = positions.at(index4);
			v2.UV = uvs.at(index4);
			v2.Normal = normals.at(index4);

			v2.UV.y = 1.0f - v2.UV.y;
			verts.push_back(v2);
			indices.push_back(vertCounter++);

			// Upper right.
			Vertex v3;
			v3.Position = positions.at(index4);
			v3.UV = uvs.at(index4);
			v3.Normal = normals.at(index4);

			v3.UV.y = 1.0f - v3.UV.y;
			verts.push_back(v3);
			indices.push_back(vertCounter++);

			// Bottom left.
			Vertex v4;
			v4.Position = positions.at(index1);
			v4.UV = uvs.at(index1);
			v4.Normal = normals.at(index1);

			v4.UV.y = 1.0f - v4.UV.y;
			verts.push_back(v4);
			indices.push_back(vertCounter++);

			// Bottom left.
			Vertex v5;
			v5.Position = positions.at(index1);
			v5.UV = uvs.at(index1);
			v5.Normal = normals.at(index1);

			v5.UV.y = 1.0f - v5.UV.y;
			verts.push_back(v5);
			indices.push_back(vertCounter++);

			// Upper left.
			Vertex v6;
			v6.Position = positions.at(index3);
			v6.UV = uvs.at(index3);
			v6.Normal = normals.at(index3);

			v6.UV.y = 1.0f - v6.UV.y;
			verts.push_back(v6);
			indices.push_back(vertCounter++);

			// Bottom left.
			Vertex v7;
			v7.Position = positions.at(index1);
			v7.UV = uvs.at(index1);
			v7.Normal = normals.at(index1);

			v7.UV.y = 1.0f - v7.UV.y;
			verts.push_back(v7);
			indices.push_back(vertCounter++);

			// Upper right.
			Vertex v8;
			v8.Position = positions.at(index4);
			v8.UV = uvs.at(index4);
			v8.Normal = normals.at(index4);

			v8.UV.y = 1.0f - v8.UV.y;
			verts.push_back(v8);
			indices.push_back(vertCounter++);

			// Upper right.
			Vertex v9;
			v9.Position = positions.at(index4);
			v9.UV = uvs.at(index4);
			v9.Normal = normals.at(index4);

			v9.UV.y = 1.0f - v9.UV.y;
			verts.push_back(v9);
			indices.push_back(vertCounter++);

			// Bottom right.
			Vertex v10;
			v10.Position = positions.at(index2);
			v10.UV = uvs.at(index2);
			v10.Normal = normals.at(index2);

			v10.UV.y = 1.0f - v10.UV.y;
			verts.push_back(v10);
			indices.push_back(vertCounter++);

			// Bottom right.
			Vertex v11;
			v10.Position = positions.at(index2);
			v10.UV = uvs.at(index2);
			v10.Normal = normals.at(index2);

			v11.UV.y = 1.0f - v11.UV.y;
			verts.push_back(v11);
			indices.push_back(vertCounter++);

			// Bottom left.
			Vertex v12;
			v12.Position = positions.at(index1);
			v12.UV = uvs.at(index1);
			v12.Normal = normals.at(index1);

			v12.UV.y = 1.0f - v12.UV.y;
			verts.push_back(v12);
			indices.push_back(vertCounter++);


			index = (j * (m_TerrainClassHeight - 1)) + i;

		}
	}


	//normalize to decrese the spikes in terrain

	for (j = 0; j<m_TerrainClassHeight; j++)
	{
		for (i = 0; i<m_TerrainClassWidth; i++)
		{
			verts.at((m_TerrainClassHeight * j) + i).Position.y /= 15.0f;
		}
	}



	numOfIndices = indices.size();
	//CalculateTangents(&verts[0], verts.size(), &indices[0], numOfIndices);
	// Create vertexBuffers
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * verts.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	// Load Data to Vertex Buffer
	D3D11_SUBRESOURCE_DATA vertData;
	vertData.pSysMem = &verts[0];
	// Create the buffer with the data
	HR(deviceObj->CreateBuffer(&vbd, &vertData, &vBuffer));

	//Create Index Buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	//Load Data to IndexBuffer
	D3D11_SUBRESOURCE_DATA indData;
	indData.pSysMem = &indices[0];
	// Create Index Buffer with the data.
	HR(deviceObj->CreateBuffer(&ibd, &indData, &iBuffer));



	// Release the bitmap image data.
	delete[] bitmapImage;
	bitmapImage = 0;

}


Mesh::~Mesh()
{
	// Release any D3D stuff that's still hanging out
	ReleaseMacro(vBuffer);
	ReleaseMacro(iBuffer);

}
void Mesh::CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices)
{
	// Reset tangents
	for (int i = 0; i < numVerts; i++)
	{
		verts[i].Tangent = XMFLOAT3(0, 0, 0);
	}

	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < numVerts;)
	{
		// Grab indices and vertices of first triangle
		unsigned int i1 = indices[i++];
		unsigned int i2 = indices[i++];
		unsigned int i3 = indices[i++];
		Vertex* v1 = &verts[i1];
		Vertex* v2 = &verts[i2];
		Vertex* v3 = &verts[i3];

		// Calculate vectors relative to triangle positions
		float x1 = v2->Position.x - v1->Position.x;
		float y1 = v2->Position.y - v1->Position.y;
		float z1 = v2->Position.z - v1->Position.z;

		float x2 = v3->Position.x - v1->Position.x;
		float y2 = v3->Position.y - v1->Position.y;
		float z2 = v3->Position.z - v1->Position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->UV.x - v1->UV.x;
		float t1 = v2->UV.y - v1->UV.y;

		float s2 = v3->UV.x - v1->UV.x;
		float t2 = v3->UV.y - v1->UV.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);

		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->Tangent.x += tx;
		v1->Tangent.y += ty;
		v1->Tangent.z += tz;

		v2->Tangent.x += tx;
		v2->Tangent.y += ty;
		v2->Tangent.z += tz;

		v3->Tangent.x += tx;
		v3->Tangent.y += ty;
		v3->Tangent.z += tz;
	}

	// Ensure all of the tangents are orthogonal to the normals
	for (int i = 0; i < numVerts; i++)
	{
		// Grab the two vectors
		XMVECTOR normal = XMLoadFloat3(&verts[i].Normal);
		XMVECTOR tangent = XMLoadFloat3(&verts[i].Tangent);

		// Use Gram-Schmidt orthogonalize
		tangent = XMVector3Normalize(
			tangent - normal * XMVector3Dot(normal, tangent));

		// Store the tangent
		XMStoreFloat3(&verts[i].Tangent, tangent);
	}
}

ID3D11Buffer* Mesh::GetIndexBuffer() {
	return iBuffer;
}
int Mesh::GetIndexCount() {
	return numOfIndices;
}
ID3D11Buffer* Mesh::GetVertexBuffer() {
	return vBuffer;
}