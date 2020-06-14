#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "macros.h"

using std::vector;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 color;
};

struct Cube
{
	// Vertices
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	// d3d pointers
	ID3D11Buffer* vertex_buffer;
	ID3D11Buffer* index_buffer;

	ID3D11InputLayout* input_layout;
	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;
};

struct HighDefinitionPlane
{
	vector<Vertex> vertices;
	vector<int> indices;

	// d3d pointers
	ID3D11Buffer* vertex_buffer;
	ID3D11Buffer* index_buffer;

	ID3D11InputLayout* input_layout;
	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;
};

class Procedural
{
public:
	// Cube
	void static ConstructCube(ID3D11Device* device, Cube& cube, const BYTE* _vs, const BYTE* _ps, int vs_size, int ps_size);
	void static CleanupCube(Cube& cube);

	// High Def Plane
	void static ConstructHighDefPlane(ID3D11Device* device, HighDefinitionPlane& cube, const BYTE* _vs, const BYTE* _ps, int vs_size, int ps_size);
	void static CleanUpHighDefPlane(HighDefinitionPlane& plane);
};

