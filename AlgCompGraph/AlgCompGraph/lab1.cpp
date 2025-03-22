#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include <DirectXTex.h>
#include "DDSTextureLoader11.h"
#include <string>
#include <vector>
#include <algorithm>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_dx11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


// DirectX variables
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11Debug* g_pDebug = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;
ID3D11InputLayout* g_pVertexLayout = nullptr;
ID3D11InputLayout* g_pSkyboxVertexLayout = nullptr;
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;
ID3D11Buffer* g_pIndexBuffer = nullptr;
ID3D11Buffer* g_pGeomBuffer = nullptr;
ID3D11Buffer* g_pVPBuffer = nullptr;
ID3D11Buffer* g_pGeomBufferSkyBox = nullptr;
ID3D11Buffer* g_pVPBufferSkyBox = nullptr;
ID3D11ShaderResourceView* g_pTextureView = nullptr;
ID3D11SamplerState* g_pSamplerState = nullptr;
ID3D11Buffer* g_pGeomBuffer2 = nullptr;

// Skybox variables
ID3D11ShaderResourceView* g_pCubemapView = nullptr;
ID3D11VertexShader* g_pSkyboxVertexShader = nullptr;
ID3D11PixelShader* g_pSkyboxPixelShader = nullptr;
ID3D11Buffer* g_pSkyboxVertexBuffer = nullptr;
ID3D11Buffer* g_pSkyboxIndexBuffer = nullptr;

ID3D11Texture2D* g_pDepthStencilTexture = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
ID3D11DepthStencilState* g_pDepthStencilState = nullptr;

ID3D11BlendState* g_pTransBlendState = nullptr;
ID3D11DepthStencilState* g_pNoDepthWriteState = nullptr;
ID3D11InputLayout* g_pTransparentInputLayout = nullptr;
ID3D11PixelShader* g_pTransparentPixelShader = nullptr;
ID3D11VertexShader* g_pTransparentVertexShader = nullptr;
ID3D11Buffer* g_pRectangleVertexBuffer = nullptr;
ID3D11Buffer* g_pRectangleIndexBuffer = nullptr;
ID3D11Buffer* g_pRectGeomBuffer = nullptr;
ID3D11RasterizerState* g_pRasterizerState = nullptr;
ID3D11Buffer* g_pBaseColorBuffer = nullptr;
//Light

ID3D11Buffer* g_pLightBuffer = nullptr;
ID3D11Buffer* g_pLightColorBuffer = nullptr;
ID3D11PixelShader* g_pLightPS = nullptr;
ID3D11ShaderResourceView* g_pNormalMapView = nullptr;
ID3D11Buffer* g_pGeomBuffer3 = nullptr;

DirectX::XMFLOAT4 g_ClearColor = { 0.2f, 0.2f, 0.4f, 1.0f };
DirectX::XMFLOAT3 g_CameraPosition = { 0.0f, 0.0f, -5.0f };
float g_RotationAngle = 0.0f;
float g_CameraMoveSpeed = 15.0f;
float g_DeltaTime = 0.0f;
bool g_Keys[256] = { false };

bool g_IsMouseDown = false;
POINT g_LastMousePos = { 0, 0 };
float g_Yaw = 0.0f;
float g_Pitch = 0.0f;
bool g_IsPaused = false;

struct GeomBuffer {
    DirectX::XMMATRIX model;
    DirectX::XMMATRIX normal;
};

struct VPBuffer {
    DirectX::XMMATRIX vp;
};

struct GeomBufferSkyBox {
    DirectX::XMMATRIX model;
    DirectX::XMFLOAT4 size;
};

struct VPBufferSkyBox {
    DirectX::XMMATRIX vp;
    DirectX::XMFLOAT4 cameraPos;
};

struct LightBufferType {
    DirectX::XMFLOAT3 lightPos;
    float pad0;
    DirectX::XMFLOAT3 lightColor;
    float pad1;
    DirectX::XMFLOAT3 ambient;
    float pad2;
    DirectX::XMFLOAT3 cameraPosition;
    float pad3;
};

// Глобальные переменные для управления светом
DirectX::XMFLOAT3 lightPosition = { 2.5f, 0.6f, 0.0f };
DirectX::XMFLOAT3 lightColor = { 1.0f, 1.0f, 1.0f };

struct TextureVertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    float u, v;
};

struct SkyboxVertex {
    float x, y, z;
};

static const TextureVertex Vertices[] = {
    // Нижняя грань (Normal: 0, -1, 0)
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), 0.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), 1.0f, 0.0f },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), 0.0f, 0.0f },

    // Верхняя грань (Normal: 0, 1, 0)
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), 1.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f },

    // Передняя грань (Normal: 0, 0, 1)
    { DirectX::XMFLOAT3(0.5f, -0.5f,  -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f },

    // Задняя грань (Normal: 0, 0, -1)
    { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), 0.0f, 1.0f },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), 1.0f, 1.0f },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), 1.0f, 0.0f },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f },

    // Левая грань (Normal: -1, 0, 0)
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), 0.0f, 1.0f },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), 1.0f, 1.0f },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), 1.0f, 0.0f },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f },

    // Правая грань (Normal: 1, 0, 0)
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), 0.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), 1.0f, 1.0f },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), 1.0f, 0.0f },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f }
};

static const WORD Indices[] = {
    // Передняя грань
    0, 2, 1, 0, 3, 2,

    // Задняя грань
    4, 6, 5, 4, 7, 6,

    // Верхняя грань
    8, 10, 9, 8, 11, 10,

    // Нижняя грань
    12, 14, 13, 12, 15, 14,

    // Левая грань
    16, 18, 17, 16, 19, 18,

    // Правая грань
    20, 22, 21, 20, 23, 22
};

SkyboxVertex skyboxVertices[] = {
    {-1.0f,  1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

    { 1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

    {-1.0f,  1.0f, -1.0f},
    { 1.0f,  1.0f, -1.0f},
    { 1.0f,  1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    { 1.0f, -1.0f,  1.0f}
};

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
};

struct RectGeomBuffer {
    DirectX::XMMATRIX model;
    DirectX::XMMATRIX normal;
};

Vertex RectangleVertices[] = {
    { DirectX::XMFLOAT3(0.0f, -0.5f, 0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.0f, 0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.0f, -0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, .0f) }
};

WORD g_RectangleIndices[] = {
    0, 1, 2,
    0, 2, 3 
};

struct RectangleInfo {
    DirectX::XMMATRIX modelMatrix;
    DirectX::XMFLOAT4 color;
    float distanceFromCamera;
};

// Матрицы для прямоугольников
DirectX::XMMATRIX modelMatrices[2] = {
    DirectX::XMMatrixTranslation(1.0f, 0.0f, 0.0f),
    DirectX::XMMatrixTranslation(1.5f, 0.0f, 0.0f)
};

DirectX::XMFLOAT4 colors[2] = {
    DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f),
    DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 0.5f)
};

struct BaseColorBufferType {
    DirectX::XMFLOAT4 baseColor;
};



LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
void CleanupDevice();
void Render();
HRESULT InitGraphics();

ID3D11ShaderResourceView* LoadTexture(const std::wstring& filePath);
ID3D11ShaderResourceView* LoadCubemap(const std::wstring& filePath);
ID3D11SamplerState* CreateSampler();

int APIENTRY WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow) {
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC, WndProc,
        0L,
        0L,
        hInstance,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        L"Laboratory work 6",
        nullptr
    };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(
        wc.lpszClassName,
        L"Laboratory work 6",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        1200, 800,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    if (FAILED(InitD3D(hWnd))) {
        CleanupDevice();
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }

    CleanupDevice();
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_MOUSEMOVE:
        if (!ImGui::GetIO().WantCaptureMouse && g_IsMouseDown) {
            POINT currentMousePos = { LOWORD(lParam), HIWORD(lParam) };
            int deltaX = currentMousePos.x - g_LastMousePos.x;
            int deltaY = currentMousePos.y - g_LastMousePos.y;

            g_Yaw += deltaX * 0.005f;
            g_Pitch += deltaY * 0.005f;

            // Ограничение угла pitch
            if (g_Pitch > DirectX::XM_PIDIV2) g_Pitch = DirectX::XM_PIDIV2;
            if (g_Pitch < -DirectX::XM_PIDIV2) g_Pitch = -DirectX::XM_PIDIV2;

            g_LastMousePos = currentMousePos;
        }
        break;

    case WM_LBUTTONDOWN:
        if (!ImGui::GetIO().WantCaptureMouse) {
            g_LastMousePos = { LOWORD(lParam), HIWORD(lParam) };
            g_IsMouseDown = true;
        }
        break;

    case WM_LBUTTONUP:
        g_IsMouseDown = false;
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case 'W':
            g_CameraPosition.x += sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z += cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'S':
            g_CameraPosition.x -= sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z -= cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'A':
            g_CameraPosition.x -= cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z += sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'D':
            g_CameraPosition.x += cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z -= sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case VK_SPACE:
            g_CameraPosition.y += g_CameraMoveSpeed * g_DeltaTime;
            break;
        case VK_CONTROL:
            g_CameraPosition.y -= g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'P':
            g_IsPaused = !g_IsPaused;
            break;
        }
        break;

    case WM_SIZE:
        if (g_pd3dDevice != nullptr && g_pSwapChain != nullptr) {
            if (g_pRenderTargetView) {
                g_pImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
                g_pRenderTargetView->Release();
                g_pRenderTargetView = nullptr;
            }
            if (g_pDepthStencilView) {
                g_pDepthStencilView->Release();
                g_pDepthStencilView = nullptr;
            }
            if (g_pDepthStencilTexture) {
                g_pDepthStencilTexture->Release();
                g_pDepthStencilTexture = nullptr;
            }
            if (g_pDepthStencilState)
            {
                g_pDepthStencilState->Release();
                g_pDepthStencilState = nullptr;
            }

            g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

            ID3D11Texture2D* pBackBuffer = nullptr;
            HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            if (FAILED(hr) || pBackBuffer == nullptr) {
                OutputDebugStringA("Failed to get back buffer from swap chain.\n");
                break;
            }

            hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
            pBackBuffer->Release();

            if (FAILED(hr)) {
                OutputDebugStringA("Failed to create render target view.\n");
                break;
            }

            D3D11_TEXTURE2D_DESC descDepth = {};
            descDepth.Width = LOWORD(lParam);
            descDepth.Height = HIWORD(lParam);
            descDepth.MipLevels = 1;
            descDepth.ArraySize = 1;
            descDepth.Format = DXGI_FORMAT_D32_FLOAT;
            descDepth.SampleDesc.Count = 1;
            descDepth.SampleDesc.Quality = 0;
            descDepth.Usage = D3D11_USAGE_DEFAULT;
            descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            descDepth.CPUAccessFlags = 0;
            descDepth.MiscFlags = 0;

            hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencilTexture);
            if (FAILED(hr)) {
                MessageBox(hWnd, L"Failed to create depth texture.", L"Error", MB_OK);
                break;
            }

            hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencilTexture, nullptr, &g_pDepthStencilView);
            if (FAILED(hr)) {
                MessageBox(hWnd, L"Failed to create depth stencil view.", L"Error", MB_OK);
                break;
            }

            D3D11_VIEWPORT vp = {};
            vp.Width = static_cast<FLOAT>(LOWORD(lParam));
            vp.Height = static_cast<FLOAT>(HIWORD(lParam));
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            g_pImmediateContext->RSSetViewports(1, &vp);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HRESULT InitD3D(HWND hWnd)
{
    HRESULT hr = S_OK;

    UINT creationFlags = 0;
#ifndef NDEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        nullptr,
        &g_pImmediateContext
    );

    if (FAILED(hr))
    {
        MessageBox(hWnd, L"Failed to create Direct3D device and swap chain.", L"Error", MB_OK);
        return hr;
    }

#ifndef NDEBUG
    hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&g_pDebug);
    if (FAILED(hr))
    {
        MessageBox(hWnd, L"Failed to get ID3D11Debug interface.", L"Error", MB_OK);
        return hr;
    }
#endif

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
        MessageBox(hWnd, L"Failed to get back buffer.", L"Error", MB_OK);
        return hr;
    }

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        MessageBox(hWnd, L"Failed to create render target view.", L"Error", MB_OK);
        return hr;
    }

    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = 800;
    descDepth.Height = 600;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencilTexture);
    if (FAILED(hr)) {
        MessageBox(hWnd, L"Failed to create depth texture.", L"Error", MB_OK);
        return hr;
    }

    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencilTexture, nullptr, &g_pDepthStencilView);
    if (FAILED(hr)) {
        MessageBox(hWnd, L"Failed to create depth stencil view.", L"Error", MB_OK);
        return hr;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    hr = g_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &g_pDepthStencilState);
    if (FAILED(hr)) {
        MessageBox(hWnd, L"Failed to create depth stencil state.", L"Error", MB_OK);
        return hr;
    }

    D3D11_VIEWPORT vp;
    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    //Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);

    if (FAILED(InitGraphics()))
    {
        MessageBox(hWnd, L"Failed to initialize graphics.", L"Error", MB_OK);
        return hr;
    }

    return S_OK;
}

HRESULT InitGraphics()
{
    HRESULT hr = S_OK;

    g_pTextureView = LoadTexture(L"Stone.dds");
    if (!g_pTextureView) {
        MessageBox(nullptr, L"Failed to load Stone.dds.", L"Error", MB_OK);
        return E_FAIL;
    }

    g_pNormalMapView = LoadTexture(L"StoneNM.dds");
    if (!g_pNormalMapView) {
        MessageBox(nullptr, L"Failed to load StoneNM.dds.", L"Error", MB_OK);
        return E_FAIL;
    }

    g_pCubemapView = LoadCubemap(L"СubeMap.dds");
    if (!g_pCubemapView) {
        return E_FAIL;
    }

    g_pSamplerState = CreateSampler();
    if (!g_pSamplerState) {
        return E_FAIL;
    }

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = Vertices;

    hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create VertexBuffer.", L"Error", MB_OK);
        return hr;
    }

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(Indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = Indices;

    hr = g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pIndexBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create IndexBuffer.", L"Error", MB_OK);
        return hr;
    }

    D3D11_BUFFER_DESC rectVbDesc = {};
    rectVbDesc.Usage = D3D11_USAGE_DEFAULT;
    rectVbDesc.ByteWidth = sizeof(RectangleVertices);
    rectVbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA rectVbData = {};
    rectVbData.pSysMem = RectangleVertices;

    hr = g_pd3dDevice->CreateBuffer(&rectVbDesc, &rectVbData, &g_pRectangleVertexBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create rectangle vertex buffer.", L"Error", MB_OK);
        return hr;
    }

    D3D11_BUFFER_DESC rectIbDesc = {};
    rectIbDesc.Usage = D3D11_USAGE_DEFAULT;
    rectIbDesc.ByteWidth = sizeof(g_RectangleIndices);
    rectIbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA rectIbData = {};
    rectIbData.pSysMem = g_RectangleIndices;

    hr = g_pd3dDevice->CreateBuffer(&rectIbDesc, &rectIbData, &g_pRectangleIndexBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create rectangle index buffer.", L"Error", MB_OK);
        return hr;
    }

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(GeomBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&cbDesc, nullptr, &g_pGeomBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create GeomBuffer.", L"Error", MB_OK);
        return hr;
    }

    D3D11_BUFFER_DESC cbDesc2 = {};
    cbDesc2.Usage = D3D11_USAGE_DEFAULT;
    cbDesc2.ByteWidth = sizeof(GeomBuffer);
    cbDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc2.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&cbDesc2, nullptr, &g_pGeomBuffer2);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create GeomBuffer2.", L"Error", MB_OK);
        return hr;
    }

    D3D11_BUFFER_DESC cbDesc3 = {};
    cbDesc3.Usage = D3D11_USAGE_DEFAULT;
    cbDesc3.ByteWidth = sizeof(GeomBuffer);
    cbDesc3.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc3.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&cbDesc3, nullptr, &g_pGeomBuffer3);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create GeomBuffer2.", L"Error", MB_OK);
        return hr;
    }

    D3D11_BUFFER_DESC vpDesc = {};
    vpDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpDesc.ByteWidth = sizeof(VPBuffer);
    vpDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = g_pd3dDevice->CreateBuffer(&vpDesc, nullptr, &g_pVPBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC cbDescSky = {};
    cbDescSky.Usage = D3D11_USAGE_DEFAULT;
    cbDescSky.ByteWidth = sizeof(GeomBufferSkyBox);
    cbDescSky.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDescSky.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&cbDescSky, nullptr, &g_pGeomBufferSkyBox);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC vpDescSky = {};
    vpDescSky.Usage = D3D11_USAGE_DYNAMIC;
    vpDescSky.ByteWidth = sizeof(VPBufferSkyBox);
    vpDescSky.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpDescSky.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = g_pd3dDevice->CreateBuffer(&vpDescSky, nullptr, &g_pVPBufferSkyBox);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC rectGeomBufferDesc = {};
    rectGeomBufferDesc.ByteWidth = sizeof(RectGeomBuffer);
    rectGeomBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    rectGeomBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    rectGeomBufferDesc.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&rectGeomBufferDesc, nullptr, &g_pRectGeomBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create RectGeomBuffer.", L"Error", MB_OK);
        return hr;
    }

    //Light
    D3D11_BUFFER_DESC baseColorBufferDesc = {};
    baseColorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    baseColorBufferDesc.ByteWidth = sizeof(BaseColorBufferType);
    baseColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    baseColorBufferDesc.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&baseColorBufferDesc, nullptr, &g_pBaseColorBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create BaseColorBuffer.", L"Error", MB_OK);
        return hr;
    }


    D3D11_BUFFER_DESC lcbDesc = {};
    lcbDesc.Usage = D3D11_USAGE_DEFAULT;
    lcbDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
    lcbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lcbDesc.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&lcbDesc, nullptr, &g_pLightColorBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC lightBufferDesc = {};
    lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&lightBufferDesc, nullptr, &g_pLightBuffer);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create light buffer.", L"Error", MB_OK);
        return hr;
    }

    // Создание blend state для прозрачности
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = g_pd3dDevice->CreateBlendState(&blendDesc, &g_pTransBlendState);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create blend state.", L"Error", MB_OK);
        return hr;
    }

    // Создание depth stencil state без записи в буфер глубины
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    hr = g_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &g_pNoDepthWriteState);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create depth stencil state.", L"Error", MB_OK);
        return hr;
    }

    D3D11_RASTERIZER_DESC desc = {};
    desc.AntialiasedLineEnable = TRUE;
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_NONE;
    desc.FrontCounterClockwise = FALSE;
    desc.DepthBias = 0;
    desc.SlopeScaledDepthBias = 0.0f;
    desc.DepthBiasClamp = 0.0f;
    desc.DepthClipEnable = TRUE;
    desc.ScissorEnable = FALSE;
    desc.MultisampleEnable = FALSE;

    hr = g_pd3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState);
    if (FAILED(hr)) {
        return hr;
    }

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_4_0", flags, 0, &vsBlob, nullptr);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_4_0", flags, 0, &psBlob, nullptr);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    if (FAILED(hr))
        return hr;

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pVertexLayout);
    if (FAILED(hr))
        return hr;

    vsBlob->Release();
    psBlob->Release();

    ID3DBlob* vsBlobSkyBox = nullptr;
    ID3DBlob* psBlobSkyBox = nullptr;

    hr = D3DCompileFromFile(L"SphereVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_4_0", flags, 0, &vsBlobSkyBox, nullptr);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"SpherePixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_4_0", flags, 0, &psBlobSkyBox, nullptr);
    if (FAILED(hr))
        return hr;

    vpDescSky.ByteWidth = sizeof(skyboxVertices);
    vpDescSky.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    initData.pSysMem = skyboxVertices;
    hr = g_pd3dDevice->CreateBuffer(&vpDescSky, &initData, &g_pSkyboxVertexBuffer);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateVertexShader(vsBlobSkyBox->GetBufferPointer(), vsBlobSkyBox->GetBufferSize(), nullptr, &g_pSkyboxVertexShader);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreatePixelShader(psBlobSkyBox->GetBufferPointer(), psBlobSkyBox->GetBufferSize(), nullptr, &g_pSkyboxPixelShader);
    if (FAILED(hr))
        return hr;

    D3D11_INPUT_ELEMENT_DESC skyboxLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = g_pd3dDevice->CreateInputLayout(skyboxLayout, ARRAYSIZE(skyboxLayout), vsBlobSkyBox->GetBufferPointer(), vsBlobSkyBox->GetBufferSize(), &g_pSkyboxVertexLayout);
    if (FAILED(hr))
        return hr;

    vsBlobSkyBox->Release();
    psBlobSkyBox->Release();

    vsBlob = nullptr;
    hr = D3DCompileFromFile(L"TransparentVertexShader.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);
    if (FAILED(hr))
        return hr;

    psBlob = nullptr;
    hr = D3DCompileFromFile(L"TransparentPixelShader.hlsl", nullptr, nullptr,
        "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_pTransparentVertexShader);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pTransparentPixelShader);
    if (FAILED(hr))
        return hr;

    D3D11_INPUT_ELEMENT_DESC Rectlayout[] = {
       { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
       { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = g_pd3dDevice->CreateInputLayout(Rectlayout, ARRAYSIZE(Rectlayout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pTransparentInputLayout);
    if (FAILED(hr))
        return hr;

    vsBlob->Release();
    psBlob->Release();

    ID3DBlob* psBlobLight = nullptr;
    ID3DBlob* errorBlob = nullptr;

    hr = D3DCompileFromFile(
        L"LightPixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain",
        "ps_5_0",
        0,
        0,
        &psBlobLight,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        MessageBox(nullptr, L"Failed to compile light pixel shader.", L"Error", MB_OK);
        return hr;
    }

    hr = g_pd3dDevice->CreatePixelShader(
        psBlobLight->GetBufferPointer(),
        psBlobLight->GetBufferSize(),
        nullptr,
        &g_pLightPS
    );

    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create light pixel shader.", L"Error", MB_OK);
        psBlobLight->Release();
        return hr;
    }

    psBlobLight->Release();

    return S_OK;
}

ID3D11ShaderResourceView* LoadTexture(const std::wstring& filePath) {

    ID3D11ShaderResourceView* textureView = nullptr;
    HRESULT hr = DirectX::CreateDDSTextureFromFile(g_pd3dDevice, filePath.c_str(), nullptr, &textureView);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to load texture.", L"Error", MB_OK);
        return nullptr;
    }
    return textureView;
}

ID3D11ShaderResourceView* LoadCubemap(const std::wstring& filePath) {
    ID3D11ShaderResourceView* cubemapView = nullptr;

    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        g_pd3dDevice,
        filePath.c_str(),
        nullptr,
        &cubemapView
    );

    if (FAILED(hr)) {
        std::wstring errorMessage = L"Failed to load cubemap texture. HRESULT: " + std::to_wstring(hr);
        MessageBox(nullptr, errorMessage.c_str(), L"Error", MB_OK);

        if (cubemapView) {
            cubemapView->Release();
            cubemapView = nullptr;
        }

        return nullptr;
    }

    return cubemapView;
}

ID3D11SamplerState* CreateSampler() {
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = -FLT_MAX;
    samplerDesc.MaxLOD = FLT_MAX;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    ID3D11SamplerState* samplerState = nullptr;
    HRESULT hr = g_pd3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
    if (FAILED(hr)) {
        return nullptr;
    }

    return samplerState;
}

void Render() {
    static DWORD previousTime = GetTickCount64();
    DWORD currentTime = GetTickCount64();

    g_DeltaTime = (currentTime - previousTime) / 1000.0f;
    if (g_DeltaTime > 0.1f) g_DeltaTime = 0.1f;
    previousTime = currentTime;

    g_RotationAngle += 1.0f * g_DeltaTime;
    if (g_RotationAngle > DirectX::XM_2PI) {
        g_RotationAngle -= DirectX::XM_2PI;
    }

    g_pImmediateContext->RSSetState(g_pRasterizerState);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, reinterpret_cast<const float*>(&g_ClearColor));
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(300, 125));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("Light Controls");

    ImGui::SliderFloat("Light X", &lightPosition.x, -10.0f, 10.0f);
    ImGui::SliderFloat("Light Y", &lightPosition.y, -10.0f, 10.0f);
    ImGui::SliderFloat("Light Z", &lightPosition.z, -10.0f, 10.0f);

    ImGui::ColorEdit3("Light Color", reinterpret_cast<float*>(&lightColor));

    ImGui::End();

    DirectX::XMVECTOR forward = DirectX::XMVectorSet(
        cosf(g_Pitch) * sinf(g_Yaw),
        sinf(g_Pitch),
        cosf(g_Pitch) * cosf(g_Yaw),
        0.0f
    );

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 0.0f);
    DirectX::XMVECTOR at = DirectX::XMVectorAdd(eye, forward);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    constexpr float fovAngleY = DirectX::XMConvertToRadians(60.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, 800.0f / 600.0f, 0.01f, 100.0f);

    // Обновление матриц для skybox
    VPBufferSkyBox vpBufferSkyBox;
    vpBufferSkyBox.vp = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
    vpBufferSkyBox.cameraPos = DirectX::XMFLOAT4(g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 1.0f);

    GeomBufferSkyBox geomBufferSkyBox;
    geomBufferSkyBox.model = DirectX::XMMatrixIdentity();
    geomBufferSkyBox.size = DirectX::XMFLOAT4(20.0f, 1.0f, 1.0f, 1.0f);

    g_pImmediateContext->UpdateSubresource(g_pGeomBufferSkyBox, 0, nullptr, &geomBufferSkyBox, 0, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResourceSkyBox;
    HRESULT hr = g_pImmediateContext->Map(g_pVPBufferSkyBox, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceSkyBox);
    if (SUCCEEDED(hr)) {
        memcpy(mappedResourceSkyBox.pData, &vpBufferSkyBox, sizeof(vpBufferSkyBox));
        g_pImmediateContext->Unmap(g_pVPBufferSkyBox, 0);
    }
    else {
        OutputDebugStringA("Failed to map VPBufferSkyBox.\n");
    }

    // Отрисовка skybox
    if (g_pCubemapView) {
        UINT stride = sizeof(SkyboxVertex);
        UINT offset = 0;

        g_pImmediateContext->VSSetShader(g_pSkyboxVertexShader, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pSkyboxPixelShader, nullptr, 0);
        g_pImmediateContext->PSSetShaderResources(0, 1, &g_pCubemapView);
        g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerState);
        g_pImmediateContext->IASetInputLayout(g_pSkyboxVertexLayout);

        g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pSkyboxVertexBuffer, &stride, &offset);
        g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pVPBufferSkyBox);
        g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pGeomBufferSkyBox);

        g_pImmediateContext->Draw(36, 0);
    }
    else {
        OutputDebugStringA("Skybox texture is not loaded.\n");
    }

    // Отрисовка кубов
    g_pImmediateContext->OMSetDepthStencilState(g_pDepthStencilState, 0);

    GeomBuffer geomBuffer;
    VPBuffer vpBuffer;

    geomBuffer.model = DirectX::XMMatrixRotationY(g_RotationAngle);
    vpBuffer.vp = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);

    g_pImmediateContext->UpdateSubresource(g_pGeomBuffer, 0, nullptr, &geomBuffer, 0, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(g_pImmediateContext->Map(g_pVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        memcpy(mappedResource.pData, &vpBuffer, sizeof(VPBuffer));
        g_pImmediateContext->Unmap(g_pVPBuffer, 0);
    }
    else {
        MessageBox(nullptr, L"Failed to map VPBuffer.", L"Error", MB_OK);
        return;
    }

    LightBufferType lightData;
    lightData.lightPos = lightPosition;
    lightData.lightColor = lightColor;
    lightData.ambient = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    lightData.cameraPosition = g_CameraPosition;
    g_pImmediateContext->UpdateSubresource(g_pLightBuffer, 0, nullptr, &lightData, 0, 0);

    // Отрисовка первого куба
    if (g_pTextureView) {
        UINT stride = sizeof(TextureVertex);
        UINT offset = 0;

        g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

        g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

        g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pLightBuffer);
        g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureView);
        g_pImmediateContext->PSSetShaderResources(1, 1, &g_pNormalMapView);
        g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerState);

        GeomBuffer geomBuffer1;
        geomBuffer1.model = DirectX::XMMatrixRotationY(g_RotationAngle);
        geomBuffer1.normal = DirectX::XMMatrixInverse(nullptr, geomBuffer1.model);
        geomBuffer1.normal = DirectX::XMMatrixTranspose(geomBuffer1.normal);

        g_pImmediateContext->UpdateSubresource(g_pGeomBuffer, 0, nullptr, &geomBuffer1, 0, 0);

        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pGeomBuffer);
        g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVPBuffer);

        g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_pImmediateContext->DrawIndexed(36, 0, 0);
    }

    // Отрисовка второго куба
    if (g_pTextureView) {
        UINT stride = sizeof(TextureVertex);
        UINT offset = 0;

        g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

        g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

        g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pLightBuffer);
        g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureView);
        g_pImmediateContext->PSSetShaderResources(1, 1, &g_pNormalMapView);
        g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerState);

        GeomBuffer geomBuffer2;
        geomBuffer2.model = DirectX::XMMatrixTranslation(2.5f, 0.0f, 0.0f);
        geomBuffer2.normal = DirectX::XMMatrixInverse(nullptr, geomBuffer2.model);
        geomBuffer2.normal = DirectX::XMMatrixTranspose(geomBuffer2.normal);
        g_pImmediateContext->UpdateSubresource(g_pGeomBuffer2, 0, nullptr, &geomBuffer2, 0, 0);

        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pGeomBuffer2);
        g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVPBuffer);

        g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_pImmediateContext->DrawIndexed(36, 0, 0);
    }

    // Отрисовка прямоугольников с прозрачностью
    g_pImmediateContext->OMSetBlendState(g_pTransBlendState, nullptr, 0xFFFFFFFF);
    g_pImmediateContext->OMSetDepthStencilState(g_pNoDepthWriteState, 0);

    if (g_pRectangleVertexBuffer && g_pRectangleIndexBuffer) {
        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pRectangleVertexBuffer, &stride, &offset);
        g_pImmediateContext->IASetIndexBuffer(g_pRectangleIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        g_pImmediateContext->VSSetShader(g_pTransparentVertexShader, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pTransparentPixelShader, nullptr, 0);
        g_pImmediateContext->IASetInputLayout(g_pTransparentInputLayout);

        RectangleInfo rectangles[2];
        BaseColorBufferType baseColorData;
        for (int i = 0; i < 2; ++i) {
            DirectX::XMVECTOR rectPosition = DirectX::XMVector3Transform(
                DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                modelMatrices[i]
            );

            DirectX::XMVECTOR cameraPosition = DirectX::XMVectorSet(
                g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 1.0f
            );

            DirectX::XMVECTOR distanceVector = DirectX::XMVectorSubtract(rectPosition, cameraPosition);
            float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(distanceVector));

            rectangles[i].modelMatrix = modelMatrices[i];
            rectangles[i].color = colors[i];
            rectangles[i].distanceFromCamera = distance;
        }

        std::sort(rectangles, rectangles + 2, [](const RectangleInfo& a, const RectangleInfo& b) {
            return a.distanceFromCamera > b.distanceFromCamera;
            });

        for (int i = 0; i < 2; ++i) {
            RectGeomBuffer rectGeomBuffer;
            rectGeomBuffer.model = rectangles[i].modelMatrix;
            DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixInverse(nullptr, rectangles[i].modelMatrix);
            normalMatrix = DirectX::XMMatrixTranspose(normalMatrix);
            rectGeomBuffer.normal = normalMatrix;

            g_pImmediateContext->UpdateSubresource(g_pRectGeomBuffer, 0, nullptr, &rectGeomBuffer, 0, 0);

            g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVPBuffer);
            g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pRectGeomBuffer);

            BaseColorBufferType baseColorData;
            baseColorData.baseColor = rectangles[i].color;
            g_pImmediateContext->UpdateSubresource(g_pBaseColorBuffer, 0, nullptr, &baseColorData, 0, 0);
            g_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pBaseColorBuffer);

            g_pImmediateContext->DrawIndexed(6, 0, 0);
        }
    }

    g_pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    g_pImmediateContext->OMSetDepthStencilState(nullptr, 0);



    //light
    UINT stride = sizeof(TextureVertex);
    UINT offset = 0;

    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f);
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(lightPosition.x, lightPosition.y, lightPosition.z);
    GeomBuffer geomBuffer3;
    geomBuffer3.model = scale * translation;
    geomBuffer3.normal = DirectX::XMMatrixInverse(nullptr, geomBuffer3.model);
    geomBuffer3.normal = DirectX::XMMatrixTranspose(geomBuffer3.normal);
    g_pImmediateContext->UpdateSubresource(g_pGeomBuffer3, 0, nullptr, &geomBuffer3, 0, 0);

    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pLightPS, nullptr, 0);

    
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pGeomBuffer3);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVPBuffer);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pLightBuffer);

    g_pImmediateContext->DrawIndexed(36, 0, 0);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(0, 0);
}

void CleanupDevice()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Освобождение буферов геометрии
    if (g_pGeomBuffer) g_pGeomBuffer->Release();
    if (g_pGeomBuffer2) g_pGeomBuffer2->Release();
    if (g_pGeomBuffer3) g_pGeomBuffer3->Release();
    if (g_pVPBuffer) g_pVPBuffer->Release();
    if (g_pIndexBuffer) g_pIndexBuffer->Release();

    // Освобождение шейдеров и макетов входных данных
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pSkyboxVertexLayout) g_pSkyboxVertexLayout->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();

    // Освобождение текстур и семплеров
    if (g_pTextureView) g_pTextureView->Release();
    if (g_pNormalMapView) g_pNormalMapView->Release();
    if (g_pSamplerState) g_pSamplerState->Release();

    // Освобождение буферов глубины
    if (g_pDepthStencilView) g_pDepthStencilView->Release();
    if (g_pDepthStencilTexture) g_pDepthStencilTexture->Release();
    if (g_pDepthStencilState) g_pDepthStencilState->Release();

    // Освобождение состояний смешивания и растеризации
    if (g_pTransBlendState) g_pTransBlendState->Release();
    if (g_pNoDepthWriteState) g_pNoDepthWriteState->Release();
    if (g_pRasterizerState) g_pRasterizerState->Release();

    // Освобождение прозрачных объектов
    if (g_pTransparentInputLayout) g_pTransparentInputLayout->Release();
    if (g_pTransparentPixelShader) g_pTransparentPixelShader->Release();
    if (g_pTransparentVertexShader) g_pTransparentVertexShader->Release();
    if (g_pRectangleVertexBuffer) g_pRectangleVertexBuffer->Release();
    if (g_pRectangleIndexBuffer) g_pRectangleIndexBuffer->Release();
    if (g_pRectGeomBuffer) g_pRectGeomBuffer->Release();
    if (g_pBaseColorBuffer) g_pBaseColorBuffer->Release();

    // Освобождение skybox
    if (g_pCubemapView) g_pCubemapView->Release();
    if (g_pSkyboxVertexShader) g_pSkyboxVertexShader->Release();
    if (g_pSkyboxPixelShader) g_pSkyboxPixelShader->Release();
    if (g_pSkyboxVertexBuffer) g_pSkyboxVertexBuffer->Release();
    if (g_pSkyboxIndexBuffer) g_pSkyboxIndexBuffer->Release();
    if (g_pGeomBufferSkyBox) g_pGeomBufferSkyBox->Release();
    if (g_pVPBufferSkyBox) g_pVPBufferSkyBox->Release();

    // Освобождение световых ресурсов
    if (g_pLightBuffer) g_pLightBuffer->Release();
    if (g_pLightColorBuffer) g_pLightColorBuffer->Release();
    if (g_pLightPS) g_pLightPS->Release();

    // Освобождение основных ресурсов Direct3D
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    // Освобождение отладчика   
    if (g_pDebug) g_pDebug->Release();
}