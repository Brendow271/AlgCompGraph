#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "Resource.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

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
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;

ID3D11Buffer* g_pIndexBuffer = nullptr;
ID3D11Buffer* g_pGeomBuffer = nullptr;
ID3D11Buffer* g_pVPBuffer = nullptr;

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

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

Vertex Vertices[] = {
    { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
    { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
    { DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }
};

WORD Indices[] = {
    0, 1, 2,
    0, 2, 3,

    4, 6, 5,
    4, 7, 6,

    4, 5, 1,
    4, 1, 0,

    3, 2, 6,
    3, 6, 7,

    1, 5, 6,
    1, 6, 2,

    4, 0, 3,
    4, 3, 7
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
void CleanupDevice();
void Render();
HRESULT InitGraphics();

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
        L"Laboratory work 3",
        nullptr
    };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(
        wc.lpszClassName,
        L"Laboratory work 3",
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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pVertexLayout);
    vsBlob->Release();
    psBlob->Release();
    if (FAILED(hr))
        return hr;

    return S_OK;
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

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

    
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
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pDebug) g_pDebug->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}