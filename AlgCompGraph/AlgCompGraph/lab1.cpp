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
    DirectX::XMFLOAT3 cameraPos;
};

struct TextureVertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
};

static const TextureVertex Vertices[] = {
    
    // Нижняя грань
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Верхняя грань
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Передняя грань
    { DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Задняя грань
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Левая грань
    { DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3( 0.5f,  0.5f,  0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },

    // Правая грань
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
    { DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) }   
};

//WORD Indices[] = {
//    0, 1, 2,
//    0, 2, 3,
//
//    4, 6, 5,
//    4, 7, 6,
//
//    4, 5, 1,
//    4, 1, 0,
//
//    3, 2, 6,
//    3, 6, 7,
//
//    1, 5, 6,
//    1, 6, 2,
//
//    4, 0, 3,
//    4, 3, 7
//};

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



LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
void CleanupDevice();
void Render();
HRESULT InitGraphics();

ID3D11ShaderResourceView* LoadTexture(const std::wstring& filePath);
ID3D11ShaderResourceView* LoadCubemap();
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
    case WM_KEYDOWN:
        switch (wParam) {
        case 'W':
            g_CameraPosition.z += g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'S':
            g_CameraPosition.z -= g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'A':
            g_CameraPosition.x -= g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'D':
            g_CameraPosition.x += g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'Q':
            g_CameraPosition.y += g_CameraMoveSpeed * g_DeltaTime;
            break;
        case 'E':
            g_CameraPosition.y -= g_CameraMoveSpeed * g_DeltaTime;
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
    g_pCubemapView = LoadCubemap();
    if (!g_pCubemapView) {
        return E_FAIL;
    }

    // Создание семплера
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
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(Indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = Indices;

    hr = g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pIndexBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(GeomBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&cbDesc, nullptr, &g_pGeomBuffer);
    if (FAILED(hr))
        return hr;

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
    vpDescSky.Usage = D3D11_USAGE_DEFAULT;
    vpDescSky.ByteWidth = sizeof(VPBufferSkyBox);
    vpDescSky.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpDescSky.CPUAccessFlags = 0;

    hr = g_pd3dDevice->CreateBuffer(&vpDescSky, nullptr, &g_pVPBufferSkyBox);
    if (FAILED(hr))
        return hr;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;

    UINT flags = 0;

#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

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
    
    // Compile skybox shaders
    hr = D3DCompileFromFile(L"SphereVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_4_0", flags, 0, &vsBlob, nullptr);
    if (FAILED(hr))
        return hr;

    hr = D3DCompileFromFile(L"SpherePixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_4_0", flags, 0, &psBlob, nullptr);
    if (FAILED(hr))
        return hr;

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    //WORD skyboxIndices[] = {
    //    // Индексы для граней куба
    //    0, 1, 2, 2, 1, 3,
    //    4, 5, 6, 6, 5, 7,
    //    0, 2, 6, 6, 4, 0,
    //    1, 3, 7, 7, 5, 1,
    //    2, 3, 7, 7, 6, 2,
    //    0, 1, 5, 5, 4, 0
    //};

    WORD skyboxIndices[] = {
        // Передняя грань
        0, 1, 2, 2, 1, 3,

        // Задняя грань
        4, 5, 6, 6, 5, 7,

        // Нижняя грань
        8, 9, 10, 10, 11, 8,

        // Верхняя грань
        12, 13, 14, 14, 15, 12,

        // Правая грань
        16, 17, 18, 18, 19, 16,

        // Левая грань
        20, 21, 22, 22, 23, 20
    };

    bd.ByteWidth = sizeof(skyboxVertices);
    initData.pSysMem = skyboxVertices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pSkyboxVertexBuffer);
    if (FAILED(hr))
        return hr;

    ibd.ByteWidth = sizeof(skyboxIndices);
    iinitData.pSysMem = skyboxIndices;
    hr = g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pSkyboxIndexBuffer);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_pSkyboxVertexShader);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pSkyboxPixelShader);
    if (FAILED(hr))
        return hr;

    D3D11_INPUT_ELEMENT_DESC skyboxLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = g_pd3dDevice->CreateInputLayout(skyboxLayout, ARRAYSIZE(skyboxLayout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pSkyboxVertexLayout);
    if (FAILED(hr))
        return hr;

    vsBlob->Release();
    psBlob->Release();

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

ID3D11ShaderResourceView* LoadCubemap() {
    ID3D11ShaderResourceView* cubemapView = nullptr;
    ID3D11Texture2D* cubemapTexture = nullptr;

    const std::wstring textureNames[6] = {
        L"px.dds", L"nx.dds",
        L"py.dds", L"ny.dds",
        L"pz.dds", L"nz.dds"
    };

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = 480; // Assuming all textures are 1024x1024
    textureDesc.Height = 480;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 6;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    std::vector<D3D11_SUBRESOURCE_DATA> subresources(6);

    for (int i = 0; i < 6; ++i) {

        //ID3D11Texture2D* texture = nullptr;
        DirectX::TexMetadata info;
        DirectX::ScratchImage image;

        HRESULT hr = DirectX::LoadFromDDSFile(textureNames[i].c_str(), DirectX::DDS_FLAGS_NONE, &info, image);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to load cubemap texture.", L"Error", MB_OK);
            return nullptr;
        }
      
        D3D11_TEXTURE2D_DESC desc;
        //texture->GetDesc(&desc);
        subresources[i].pSysMem = image.GetPixels(); //?
        subresources[i].SysMemPitch = info.width;
        subresources[i].SysMemSlicePitch = 0;

        //texture->Release();
    }

    HRESULT hr = g_pd3dDevice->CreateTexture2D(&textureDesc, subresources.data(), &cubemapTexture);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create cubemap texture.", L"Error", MB_OK);
        return nullptr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = 1;

    hr = g_pd3dDevice->CreateShaderResourceView(cubemapTexture, &srvDesc, &cubemapView);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create cubemap shader resource view.", L"Error", MB_OK);
        cubemapTexture->Release();
        return nullptr;
    }

    cubemapTexture->Release();
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

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, reinterpret_cast<const float*>(&g_ClearColor));

    GeomBuffer geomBuffer;
    VPBuffer vpBuffer;

    geomBuffer.model = DirectX::XMMatrixRotationY(g_RotationAngle);

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(g_CameraPosition.x, g_CameraPosition.y, g_CameraPosition.z, 0.0f);
    DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    constexpr float fovAngleY = DirectX::XMConvertToRadians(60.0f);
    vpBuffer.vp = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixLookAtLH(eye, at, up),
        DirectX::XMMatrixPerspectiveFovLH(fovAngleY, 800.0f / 600.0f, 0.01f, 100.0f)
    );
    

    g_pImmediateContext->UpdateSubresource(g_pGeomBuffer, 0, nullptr, &geomBuffer, 0, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    g_pImmediateContext->Map(g_pVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &vpBuffer, sizeof(VPBuffer));
    g_pImmediateContext->Unmap(g_pVPBuffer, 0);

    

    VPBufferSkyBox vpBufferSkyBox;
    vpBufferSkyBox.vp = vpBuffer.vp;
    vpBufferSkyBox.cameraPos = g_CameraPosition;

    GeomBufferSkyBox geomBufferSkyBox;
    geomBufferSkyBox.model = DirectX::XMMatrixIdentity();
    geomBufferSkyBox.size = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    g_pImmediateContext->UpdateSubresource(g_pGeomBufferSkyBox, 0, nullptr, &geomBufferSkyBox, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pVPBufferSkyBox, 0, nullptr, &vpBufferSkyBox, 0, 0);

    UINT stride = 12;
    UINT offset = 0;

    // Render Skybox
    g_pImmediateContext->VSSetShader(g_pSkyboxVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pSkyboxPixelShader, nullptr, 0);
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pCubemapView);
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerState);
    g_pImmediateContext->IASetInputLayout(g_pSkyboxVertexLayout);

    // Set skybox geometry and draw
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pSkyboxVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pSkyboxIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pGeomBufferSkyBox);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVPBufferSkyBox);

    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_pImmediateContext->DrawIndexed(36, 0, 0);
   // g_pImmediateContext->Draw(36,0);


    //Cube
    stride = sizeof(TextureVertex);
    offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

    // Привязка текстуры и семплера
    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureView);
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerState);

    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pGeomBuffer);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pVPBuffer);

    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_pImmediateContext->DrawIndexed(36, 0, 0);


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