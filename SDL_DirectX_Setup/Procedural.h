#pragma once

#include <DirectXMath.h>
#include <vector>

using std::vector;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 color;
};

struct HighDefinitionPlane
{
	vector<Vertex> vertices;
	vector<int> indices;
};

class Procedural
{
public:


private:

};

