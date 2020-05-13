#include <iostream>
#include <vector>

#include "SDL.h"
#include "SDL_syswm.h"

// Shader Headers
#include "shaders/cube_vs.csh"
#include "shaders/cube_ps.csh"
// Shader Headers

#include "Camera.h"
#include "macros.h"

// Namespaces
using namespace DirectX;

// Defines
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 800

// D3D11 Stuff
ID3D11Device* g_Device;
IDXGISwapChain* g_Swapchain;
ID3D11DeviceContext* g_DeviceContext;
float g_aspectRatio = 1;
float g_rotation = 0;
bool g_fullscreen = false;

// States
ID3D11RasterizerState* rasterizerStateDefault;
ID3D11RasterizerState* rasterizerStateWireframe;

// Shader variables
ID3D11Buffer* constantBuffer;

// Z buffer
ID3D11Texture2D* zBuffer;
ID3D11DepthStencilView* depthStencil;

// For drawing  -> New stuff right here
ID3D11RenderTargetView* g_RenderTargetView;
D3D11_VIEWPORT g_viewport;
// D3D11 Stuff

// Custom struct WorldViewProjection
struct WorldViewProjection {
	XMFLOAT4X4 WorldMatrix;
	XMFLOAT4X4 ViewMatrix;
	XMFLOAT4X4 ProjectionMatrix;
} WORLD;

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

FPSCamera camera;
Cube cube;
// Custom stuff

// Forward declarations
void ConstructCube(Cube& cube, const BYTE* _vs, const BYTE* _ps, int vs_size, int ps_size);
void CleanupCube(Cube& cube);

int main(int argc, char** argv)
{
	// Members
	bool RUNNING = true;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		std::cout << "Initialization failed" << std::endl;
	}

	// Create window
	SDL_Window* m_window = SDL_CreateWindow("SDLTemplate",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	// Full screen
	if(g_fullscreen)
		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);

	if (!m_window)
	{
		std::cout << "Window initialization failed\n";
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_window, &wmInfo);
	HWND hWnd = wmInfo.info.win.window;

	// D3d11 code here
	RECT rect;
	GetClientRect(hWnd, &rect);

	// Attach d3d to the window
	D3D_FEATURE_LEVEL DX11 = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swap;
	ZeroMemory(&swap, sizeof(DXGI_SWAP_CHAIN_DESC));
	swap.BufferCount = 1;
	swap.OutputWindow = hWnd;
	swap.Windowed = true;
	swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap.BufferDesc.Width = rect.right - rect.left;
	swap.BufferDesc.Height = rect.bottom - rect.top;
	swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap.SampleDesc.Count = 1;

	g_aspectRatio = swap.BufferDesc.Width / (float)swap.BufferDesc.Height;

	HRESULT result;

	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &DX11, 1, D3D11_SDK_VERSION, &swap, &g_Swapchain, &g_Device, 0, &g_DeviceContext);
	assert(!FAILED(result));

	ID3D11Resource* backbuffer;
	result = g_Swapchain->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer);
	result = g_Device->CreateRenderTargetView(backbuffer, NULL, &g_RenderTargetView);
	assert(!FAILED(result));

	// Release the resource to decrement the counter by one
	// This is necessary to keep the buffer from leaking memory
	backbuffer->Release();

	// Setup viewport
	g_viewport.Width = swap.BufferDesc.Width;
	g_viewport.Height = swap.BufferDesc.Height;
	g_viewport.TopLeftY = g_viewport.TopLeftX = 0;
	g_viewport.MinDepth = 0;
	g_viewport.MaxDepth = 1;
	// D3d11 code here

	// Rasterizer states
	D3D11_RASTERIZER_DESC rdesc;
	ZeroMemory(&rdesc, sizeof(D3D11_RASTERIZER_DESC));
	rdesc.FrontCounterClockwise = false;
	rdesc.DepthBiasClamp = 1;
	rdesc.DepthBias = rdesc.SlopeScaledDepthBias = 0;
	rdesc.DepthClipEnable = true;
	rdesc.FillMode = D3D11_FILL_SOLID;
	rdesc.CullMode = D3D11_CULL_BACK;
	rdesc.AntialiasedLineEnable = false;
	rdesc.MultisampleEnable = false;

	result = g_Device->CreateRasterizerState(&rdesc, &rasterizerStateDefault);
	ASSERT_HRESULT_SUCCESS(result);

	// Wire frame Rasterizer State
	ZeroMemory(&rdesc, sizeof(D3D11_RASTERIZER_DESC));
	rdesc.FillMode = D3D11_FILL_WIREFRAME;
	rdesc.CullMode = D3D11_CULL_NONE;
	rdesc.DepthClipEnable = true;

	result = g_Device->CreateRasterizerState(&rdesc, &rasterizerStateWireframe);
	ASSERT_HRESULT_SUCCESS(result);

	g_DeviceContext->RSSetState(rasterizerStateDefault);

	// Initialize camera
	camera.SetPosition(XMFLOAT3(0, 1.5, -5));
	camera.Rotate(0, -20);
	camera.SetFOV(45);

	ConstructCube(cube, cube_vs, cube_ps, sizeof(cube_vs), sizeof(cube_ps));

	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Create constant buffer
	D3D11_BUFFER_DESC bDesc;
	D3D11_SUBRESOURCE_DATA subdata;
	ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&subdata, sizeof(D3D11_SUBRESOURCE_DATA));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.ByteWidth = sizeof(WorldViewProjection);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	result = g_Device->CreateBuffer(&bDesc, nullptr, &constantBuffer);
	ASSERT_HRESULT_SUCCESS(result);

	// Z buffer 
	D3D11_TEXTURE2D_DESC zDesc;
	ZeroMemory(&zDesc, sizeof(zDesc));
	zDesc.ArraySize = 1;
	zDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	zDesc.Width = swap.BufferDesc.Width;
	zDesc.Height = swap.BufferDesc.Height;
	zDesc.Usage = D3D11_USAGE_DEFAULT;
	zDesc.Format = DXGI_FORMAT_D32_FLOAT;
	zDesc.MipLevels = 1;
	zDesc.SampleDesc.Count = 1;

	result = g_Device->CreateTexture2D(&zDesc, nullptr, &zBuffer);
	ASSERT_HRESULT_SUCCESS(result);
	result = g_Device->CreateDepthStencilView(zBuffer, nullptr, &depthStencil);
	ASSERT_HRESULT_SUCCESS(result);

	// Main loop
	while (RUNNING)
	{
		// Event check
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// Input handling
			if (event.type == SDL_QUIT)
				RUNNING = false;
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
					RUNNING = false;
				else if (event.key.keysym.scancode == SDL_SCANCODE_F && g_fullscreen)
				{
					g_fullscreen = false;
					SDL_SetWindowFullscreen(m_window, 0);
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_F && !g_fullscreen)
				{
					g_fullscreen = true;
					SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
				}
			}
			// Input handling
		}

		// Rotation
		g_rotation += .001;
		if (g_rotation > 360) g_rotation = 0;

		// Output merger
		ID3D11RenderTargetView* tempRTV[] = { g_RenderTargetView };
		g_DeviceContext->OMSetRenderTargets(ARRAYSIZE(tempRTV), tempRTV, depthStencil);

		float color[4] = { 0, 0, 0, 1 };
		g_DeviceContext->ClearRenderTargetView(g_RenderTargetView, color);
		g_DeviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1, 0);

		g_DeviceContext->RSSetViewports(1, &g_viewport);

		// Draw the cube
		// World
		//XMMATRIX temp = XMMatrixIdentity();
		XMMATRIX temp = XMMatrixRotationY(g_rotation);
		XMStoreFloat4x4(&WORLD.WorldMatrix, temp);

		// View
		camera.GetViewMatrix(temp);
		XMStoreFloat4x4(&WORLD.ViewMatrix, temp);

		// Proj
		temp = XMMatrixPerspectiveFovLH(camera.GetFOV(), g_aspectRatio, 0.1f, 1000);
		XMStoreFloat4x4(&WORLD.ProjectionMatrix, temp);

		// Send the matrix to constant buffer
		D3D11_MAPPED_SUBRESOURCE gpuBuffer;
		HRESULT result = g_DeviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		memcpy(gpuBuffer.pData, &WORLD, sizeof(WORLD));
		g_DeviceContext->Unmap(constantBuffer, 0);
		// Connect constant buffer to the pipeline
		ID3D11Buffer* teapotCBuffers[] = { constantBuffer };
		g_DeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(teapotCBuffers), teapotCBuffers);

		UINT teapotstrides[] = { sizeof(Vertex) };
		UINT teapotoffsets[] = { 0 };
		ID3D11Buffer* teapotVertexBuffers[] = { cube.vertex_buffer };
		g_DeviceContext->IASetVertexBuffers(0, ARRAYSIZE(teapotVertexBuffers), teapotVertexBuffers, teapotstrides, teapotoffsets);
		g_DeviceContext->IASetIndexBuffer(cube.index_buffer, DXGI_FORMAT_R32_UINT, 0);
		g_DeviceContext->VSSetShader(cube.vertex_shader, 0, 0);
		g_DeviceContext->PSSetShader(cube.pixel_shader, 0, 0);
		g_DeviceContext->IASetInputLayout(cube.input_layout);

		g_DeviceContext->DrawIndexed(cube.indices.size(), 0, 0);
		// Draw the cube

		g_Swapchain->Present(0, 0);
	}

	// Cleanup
	CleanupCube(cube);

	// Delete
	D3DSAFERELEASE(g_Swapchain);
	D3DSAFERELEASE(g_DeviceContext);
	D3DSAFERELEASE(g_Device);
	D3DSAFERELEASE(g_RenderTargetView);

	D3DSAFERELEASE(rasterizerStateDefault);
	D3DSAFERELEASE(rasterizerStateWireframe);

	D3DSAFERELEASE(constantBuffer);
	D3DSAFERELEASE(zBuffer);
	D3DSAFERELEASE(depthStencil);
	// Delete

	// SDL shutdown
	if(m_window)
		SDL_DestroyWindow(m_window);
	SDL_Quit();

	return EXIT_SUCCESS;
}

void CleanupCube(Cube& cube)
{
	D3DSAFERELEASE(cube.vertex_shader);
	D3DSAFERELEASE(cube.pixel_shader);
	D3DSAFERELEASE(cube.vertex_buffer);
	D3DSAFERELEASE(cube.index_buffer);
	D3DSAFERELEASE(cube.input_layout);
}

void ConstructCube(Cube& cube, const BYTE* _vs, const BYTE* _ps, int vs_size, int ps_size)
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

	HRESULT result = g_Device->CreateBuffer(&bufferDesc, &subdata, &cube.vertex_buffer);
	ASSERT_HRESULT_SUCCESS(result);

	// Index Buffer
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(int) * cube.indices.size();

	subdata.pSysMem = cube.indices.data();
	result = g_Device->CreateBuffer(&bufferDesc, &subdata, &cube.index_buffer);
	ASSERT_HRESULT_SUCCESS(result);

	// Load shaders
	result = g_Device->CreateVertexShader(_vs, vs_size, nullptr, &cube.vertex_shader);
	ASSERT_HRESULT_SUCCESS(result);
	result = g_Device->CreatePixelShader(_ps, ps_size, nullptr, &cube.pixel_shader);
	ASSERT_HRESULT_SUCCESS(result);

	// Make input layout for vertex buffer
	D3D11_INPUT_ELEMENT_DESC tempInputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	result = g_Device->CreateInputLayout(tempInputElementDesc, ARRAYSIZE(tempInputElementDesc), _vs, vs_size, &cube.input_layout);
	ASSERT_HRESULT_SUCCESS(result);
}