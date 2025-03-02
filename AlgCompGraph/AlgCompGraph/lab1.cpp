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

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

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

// Skybox variables
ID3D11ShaderResourceView* g_pCubemapView = nullptr;
ID3D11VertexShader* g_pSkyboxVertexShader = nullptr;
ID3D11PixelShader* g_pSkyboxPixelShader = nullptr;
ID3D11Buffer* g_pSkyboxVertexBuffer = nullptr;
ID3D11Buffer* g_pSkyboxIndexBuffer = nullptr;

DirectX::XMFLOAT4 g_ClearColor = { 0.2f, 0.2f, 0.4f, 1.0f };
DirectX::XMFLOAT3 g_CameraPosition = { 0.0f, 0.0f, -5.0f };
float g_RotationAngle = 0.0f;
float g_CameraMoveSpeed = 15.0f;
float g_DeltaTime = 0.0f;
bool g_Keys[256] = { false };

// Глобальные переменные для управления мышью
bool g_IsMouseDown = false; // Состояние левой кнопки мыши
POINT g_LastMousePos = { 0, 0 }; // Последняя позиция мыши
float g_Yaw = 0.0f; // Угол поворота по оси Y (в радианах)
float g_Pitch = 0.0f; // Угол поворота по оси X (в радианах)
bool g_IsPaused = false;

struct GeomBuffer {
    DirectX::XMMATRIX model;
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

struct TextureVertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
};

struct SkyboxVertex {
    float x, y, z;
};

static const TextureVertex Vertices[] = {

    // Нижняя грань
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Верхняя грань
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Передняя грань
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Задняя грань
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Левая грань
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Правая грань
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) }
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
    // positions          
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

//WORD skyboxIndices[] = {
//    // Передняя грань
//    0, 1, 2, 2, 1, 3,

//    // Задняя грань
//    4, 5, 6, 6, 5, 7,

//    // Нижняя грань
//    8, 9, 10, 10, 11, 8,

//    // Верхняя грань
//    12, 13, 14, 14, 15, 12,

//    // Правая грань
//    16, 17, 18, 18, 19, 16,

//    // Левая грань
//    20, 21, 22, 22, 23, 20
//};

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
        L"Laboratory work 4",
        nullptr
    };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(
        wc.lpszClassName,
        L"Laboratory work 4",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        800, 600,
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_MOUSEMOVE:
        if (g_IsMouseDown) {
            // Получаем текущую позицию мыши
            POINT currentMousePos = { LOWORD(lParam), HIWORD(lParam) };

            // Вычисляем разницу в движении мыши
            int deltaX = currentMousePos.x - g_LastMousePos.x;
            int deltaY = currentMousePos.y - g_LastMousePos.y;

            // Обновляем углы камеры
            g_Yaw += deltaX * 0.005f; // Чувствительность по оси Y
            g_Pitch += deltaY * 0.005f; // Чувствительность по оси X

            // Ограничиваем угол Pitch, чтобы камера не переворачивалась
            if (g_Pitch > DirectX::XM_PIDIV2) g_Pitch = DirectX::XM_PIDIV2;
            if (g_Pitch < -DirectX::XM_PIDIV2) g_Pitch = -DirectX::XM_PIDIV2;

            // Обновляем последнюю позицию мыши
            g_LastMousePos = currentMousePos;
        }
        break;

    case WM_LBUTTONDOWN:
        // Запоминаем позицию мыши и состояние кнопки
        g_LastMousePos = { LOWORD(lParam), HIWORD(lParam) };
        g_IsMouseDown = true;
        break;

    case WM_LBUTTONUP:
        // Сбрасываем состояние кнопки
        g_IsMouseDown = false;
        break;
    case WM_KEYDOWN:
        // Обработка клавиш для движения камеры
        switch (wParam) {
        case 'W': // Движение вперед
            g_CameraPosition.x += sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z += cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'S': // Движение назад
            g_CameraPosition.x -= sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z -= cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'A': // Движение влево
            g_CameraPosition.x -= cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z += sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'D': // Движение вправо
            g_CameraPosition.x += cosf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            g_CameraPosition.z -= sinf(g_Yaw) * g_CameraMoveSpeed * g_DeltaTime;
            break;
        case VK_SPACE: // Движение вверх
            g_CameraPosition.y += g_CameraMoveSpeed * g_DeltaTime;
            break;
        case VK_CONTROL: // Движение вниз
            g_CameraPosition.y -= g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'P': // Пауза/продолжение
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

            g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
            RECT rc;
            GetClientRect(hWnd, &rc);
            D3D11_VIEWPORT vp = {};
            vp.Width = static_cast<FLOAT>(rc.right - rc.left);
            vp.Height = static_cast<FLOAT>(rc.bottom - rc.top);
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

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    D3D11_VIEWPORT vp;
    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

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

    // Загрузка текстуры
    g_pTextureView = LoadTexture(L"Puppy.dds");
    if (!g_pTextureView) {
        return E_FAIL;
    }

    // Загрузка cubemap текстуры
    g_pCubemapView = LoadCubemap(L"СubeMap.dds");
    if (!g_pCubemapView) {
        return E_FAIL;
    }

    // Создание семплера
    g_pSamplerState = CreateSampler();
    if (!g_pSamplerState) {
        return E_FAIL;
    }

    // Создание вершинного буфера для куба
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

    // Создание индексного буфера для куба
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

    // Создание константного буфера GeomBuffer
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

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;

    UINT flags = 0;

#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

    // Компиляция шейдеров для основного объекта (куба)
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
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pVertexLayout);
    if (FAILED(hr))
        return hr;

    vsBlob->Release();
    psBlob->Release();

    // Компиляция шейдеров для skybox
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

    /*ibd.ByteWidth = sizeof(skyboxIndices);
    iinitData.pSysMem = skyboxIndices;
    hr = g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pSkyboxIndexBuffer);
    if (FAILED(hr))
        return hr;*/

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

    // Загрузка текстуры Cubemap из файла
    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        g_pd3dDevice, // Устройство Direct3D
        filePath.c_str(), // Путь к файлу
        nullptr, // Ресурс текстуры (не используется)
        &cubemapView // Шейдерный ресурс
    );

    if (FAILED(hr)) {
        std::wstring errorMessage = L"Failed to load cubemap texture. HRESULT: " + std::to_wstring(hr);
        MessageBox(nullptr, errorMessage.c_str(), L"Error", MB_OK);

        // Освобождаем ресурс, если он был частично создан
        if (cubemapView) {
            cubemapView->Release();
            cubemapView = nullptr;
        }

        return nullptr;
    }

    return cubemapView; // Возвращаем указатель на шейдерный ресурс
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

    // Очистка рендер-таргета
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, reinterpret_cast<const float*>(&g_ClearColor));

    // Вычисляем направление камеры на основе углов Yaw и Pitch
    DirectX::XMVECTOR forward = DirectX::XMVectorSet(
        cosf(g_Pitch) * sinf(g_Yaw),
        sinf(g_Pitch),
        cosf(g_Pitch) * cosf(g_Yaw),
        0.0f
    );

    // Позиция камеры
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 0.0f);
    DirectX::XMVECTOR at = DirectX::XMVectorAdd(eye, forward); // Направление камеры
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    constexpr float fovAngleY = DirectX::XMConvertToRadians(60.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, 800.0f / 600.0f, 0.01f, 100.0f);

    // Обновление матриц для skybox
    VPBufferSkyBox vpBufferSkyBox;
    vpBufferSkyBox.vp = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);  // Матрица вида
    vpBufferSkyBox.cameraPos = DirectX::XMFLOAT4(g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 1.0f);  // Матрица проекции

    GeomBufferSkyBox geomBufferSkyBox;
    geomBufferSkyBox.model = DirectX::XMMatrixIdentity(); // Матрица модели для skybox (без изменений)
    geomBufferSkyBox.size = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Масштаб skybox (1.0f — без изменений)

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

        g_pImmediateContext->Draw(36, 0); // Отрисовка 36 вершин (6 граней по 2 треугольника)
    }
    else {
        OutputDebugStringA("Skybox texture is not loaded.\n");
    }

    // Обновление матриц для основного объекта (куба)
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

    // Отрисовка куба
    if (g_pTextureView) {
        UINT stride = sizeof(TextureVertex);
        UINT offset = 0;
        g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

        g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
        g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

        g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureView);
        g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerState);

        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pGeomBuffer);
        g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVPBuffer);

        g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        g_pImmediateContext->DrawIndexed(36, 0, 0);
    }
    else {
        OutputDebugStringA("Cube texture is not loaded.\n");
    }

    // Отображение результата
    g_pSwapChain->Present(0, 0);
}

void CleanupDevice()
{
    if (g_pGeomBuffer) g_pGeomBuffer->Release();
    if (g_pVPBuffer) g_pVPBuffer->Release();
    if (g_pIndexBuffer) g_pIndexBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pSkyboxVertexLayout) g_pSkyboxVertexLayout->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pDebug) g_pDebug->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
    if (g_pTextureView) g_pTextureView->Release();
    if (g_pSamplerState) g_pSamplerState->Release();
    if (g_pCubemapView) g_pCubemapView->Release();
    if (g_pSkyboxVertexShader) g_pSkyboxVertexShader->Release();
    if (g_pSkyboxPixelShader) g_pSkyboxPixelShader->Release();
    if (g_pSkyboxVertexBuffer) g_pSkyboxVertexBuffer->Release();
    if (g_pSkyboxIndexBuffer) g_pSkyboxIndexBuffer->Release();
}