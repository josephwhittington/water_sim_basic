#include "Procedural.h"

void Procedural::ConstructCube(ID3D11Device* device, Cube& cube, const BYTE* _vs, const BYTE* _ps, int vs_size, int ps_size)
{
	// Construct vertices
	Vertex vert;
	float HUL = .5; //  Half unity length

	// Top forward left
	vert.position = XMFLOAT3(-HUL, HUL, -HUL);
	vert.color = XMFLOAT3(0, 0, 1);
	cube.vertices.push_back(vert);
	// Top forward right
	vert.position = XMFLOAT3(HUL, HUL, -HUL);
	vert.color = XMFLOAT3(0, 1, 0);
	cube.vertices.push_back(vert);
	// Top back left
	vert.position = XMFLOAT3(-HUL, HUL, HUL);
	vert.color = XMFLOAT3(1, 0, 0);
	cube.vertices.push_back(vert);
	// Top back right
	vert.position = XMFLOAT3(HUL, HUL, HUL);
	vert.color = XMFLOAT3(1, 0, 1);
	cube.vertices.push_back(vert);

	// Bottom forward left
	vert.position = XMFLOAT3(-HUL, -HUL, -HUL);
	vert.color = XMFLOAT3(1, 1, 0);
	cube.vertices.push_back(vert);
	// Bottom forward right
	vert.position = XMFLOAT3(HUL, -HUL, -HUL);
	vert.color = XMFLOAT3(0, 1, 1);
	cube.vertices.push_back(vert);
	// Bottom backward left
	vert.position = XMFLOAT3(-HUL, -HUL, HUL);
	vert.color = XMFLOAT3(1, 1, 1);
	cube.vertices.push_back(vert);
	// Bottom backward right
	vert.position = XMFLOAT3(HUL, -HUL, HUL);
	vert.color = XMFLOAT3(0, 0, 0);
	cube.vertices.push_back(vert);

	// Make the triangles with the indices
	// Front face
	cube.indices.push_back(0); cube.indices.push_back(1); cube.indices.push_back(5);
	cube.indices.push_back(0); cube.indices.push_back(5); cube.indices.push_back(4);
	// Back face
	cube.indices.push_back(2); cube.indices.push_back(7); cube.indices.push_back(3);
	cube.indices.push_back(2); cube.indices.push_back(6); cube.indices.push_back(7);
	// Top face
	cube.indices.push_back(2); cube.indices.push_back(3); cube.indices.push_back(1);
	cube.indices.push_back(2); cube.indices.push_back(1); cube.indices.push_back(0);
	// Bottom face
	cube.indices.push_back(6); cube.indices.push_back(5); cube.indices.push_back(7);
	cube.indices.push_back(6); cube.indices.push_back(4); cube.indices.push_back(5);
	// Left face
	cube.indices.push_back(2); cube.indices.push_back(0); cube.indices.push_back(4);
	cube.indices.push_back(2); cube.indices.push_back(4); cube.indices.push_back(6);
	// Right face
	cube.indices.push_back(1); cube.indices.push_back(3); cube.indices.push_back(7);
	cube.indices.push_back(1); cube.indices.push_back(7); cube.indices.push_back(5);

	// Make vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subdata;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&subdata, sizeof(D3D11_SUBRESOURCE_DATA));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex) * cube.vertices.size();
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	subdata.pSysMem = cube.vertices.data();

	HRESULT result = device->CreateBuffer(&bufferDesc, &subdata, &cube.vertex_buffer);
	ASSERT_HRESULT_SUCCESS(result);

	// Index Buffer
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(int) * cube.indices.size();

	subdata.pSysMem = cube.indices.data();
	result = device->CreateBuffer(&bufferDesc, &subdata, &cube.index_buffer);
	ASSERT_HRESULT_SUCCESS(result);

	// Load shaders
	result = device->CreateVertexShader(_vs, vs_size, nullptr, &cube.vertex_shader);
	ASSERT_HRESULT_SUCCESS(result);
	result = device->CreatePixelShader(_ps, ps_size, nullptr, &cube.pixel_shader);
	ASSERT_HRESULT_SUCCESS(result);

	// Make input layout for vertex buffer
	D3D11_INPUT_ELEMENT_DESC tempInputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	result = device->CreateInputLayout(tempInputElementDesc, ARRAYSIZE(tempInputElementDesc), _vs, vs_size, &cube.input_layout);
	ASSERT_HRESULT_SUCCESS(result);
}

void Procedural::CleanupCube(Cube& cube)
{
	D3DSAFERELEASE(cube.vertex_shader);
	D3DSAFERELEASE(cube.pixel_shader);
	D3DSAFERELEASE(cube.vertex_buffer);
	D3DSAFERELEASE(cube.index_buffer);
	D3DSAFERELEASE(cube.input_layout);
}

void Procedural::ConstructHighDefPlane(ID3D11Device* device, HighDefinitionPlane& plane, const BYTE* _vs, const BYTE* _ps, int vs_size, int ps_size)
{
	// Construct plane
	// Limits
	const int xsize = 100;
	const int zsize = 100;

	const int xadjust = xsize / 2;
	const int zadjust = zsize / 2;
	int vertCount = (xsize + 1) * (zsize + 1);

	plane.vertices.resize(vertCount);
	plane.indices.resize(xsize * zsize * 6);

	// Grid size
	XMFLOAT3 color(0, 0, 1); // Blue
	for (int z = 0, i = 0; z <= zsize; z++)
	{
		for (int x = 0; x <= xsize; x++, i++)
		{
			plane.vertices[i].position = XMFLOAT3(x - xadjust, 0, z - zadjust);
			plane.vertices[i].color = color;
		}
	}

	// Indices
	int vert = 0, indexcount = 0;
	for (int z = 0; z < zsize; z++)
	{
		for (int x = 0; x < xsize; x++)
		{
			plane.indices[indexcount + 0] = vert + 0;
			plane.indices[indexcount + 1] = vert + xsize + 1;
			plane.indices[indexcount + 2] = vert + 1;
			plane.indices[indexcount + 3] = vert + 1;
			plane.indices[indexcount + 4] = vert + xsize + 1;
			plane.indices[indexcount + 5] = vert + xsize + 2;

			vert++; indexcount += 6;
		}
		vert++;
	}

	// D3D Pointers
	// Make vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subdata;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&subdata, sizeof(D3D11_SUBRESOURCE_DATA));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex) * plane.vertices.size();
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	subdata.pSysMem = plane.vertices.data();

	HRESULT result = device->CreateBuffer(&bufferDesc, &subdata, &plane.vertex_buffer);
	ASSERT_HRESULT_SUCCESS(result);

	// Index Buffer
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(int) * plane.indices.size();

	subdata.pSysMem = plane.indices.data();
	result = device->CreateBuffer(&bufferDesc, &subdata, &plane.index_buffer);
	ASSERT_HRESULT_SUCCESS(result);

	// Load shaders
	result = device->CreateVertexShader(_vs, vs_size, nullptr, &plane.vertex_shader);
	ASSERT_HRESULT_SUCCESS(result);
	result = device->CreatePixelShader(_ps, ps_size, nullptr, &plane.pixel_shader);
	ASSERT_HRESULT_SUCCESS(result);

	// Make input layout for vertex buffer
	D3D11_INPUT_ELEMENT_DESC tempInputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	result = device->CreateInputLayout(tempInputElementDesc, ARRAYSIZE(tempInputElementDesc), _vs, vs_size, &plane.input_layout);
	ASSERT_HRESULT_SUCCESS(result);
}

void Procedural::CleanUpHighDefPlane(HighDefinitionPlane& plane)
{
	D3DSAFERELEASE(plane.vertex_shader);
	D3DSAFERELEASE(plane.pixel_shader);
	D3DSAFERELEASE(plane.vertex_buffer);
	D3DSAFERELEASE(plane.index_buffer);
	D3DSAFERELEASE(plane.input_layout);
}
