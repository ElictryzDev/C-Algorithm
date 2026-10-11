#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <filesystem>

#include "Core/ProjectManager.hpp"
#include "UI/Workspaces.hpp"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void SetupOLEDDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 5.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 6.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.ItemSpacing       = ImVec2(10, 8);
    style.FramePadding      = ImVec2(8, 5);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.07f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_Border]                = ImVec4(0.18f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.16f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.06f, 0.08f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.08f, 0.11f, 0.15f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.07f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.05f, 0.07f, 0.09f, 0.50f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.20f, 0.25f, 0.32f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.00f, 0.80f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.00f, 0.80f, 0.92f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.13f, 0.17f, 0.23f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.00f, 0.60f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.00f, 0.45f, 0.70f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.15f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.00f, 0.60f, 0.85f, 0.70f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.00f, 0.60f, 0.85f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.13f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.00f, 0.65f, 0.90f, 0.80f);
    colors[ImGuiCol_TabSelected]           = ImVec4(0.00f, 0.50f, 0.80f, 1.00f);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"CAlgorithmClass", nullptr };
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"C-Algorithm — Visual Scripting & Data Structure Advisor (DirectX 11)", 
                              WS_OVERLAPPEDWINDOW, 100, 100, 1440, 900, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    SetupOLEDDarkTheme();

    // ── Kristal Netliginde Turkce Destekli Segoe UI Font Yukle ───────────────
    static const ImWchar turkishRanges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0100, 0x017F, // Latin Extended-A (ç, ğ, ı, ö, ş, ü, Ç, Ğ, İ, Ö, Ş, Ü)
        0,
    };
    if (std::filesystem::exists("C:\\Windows\\Fonts\\segoeui.ttf")) {
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 17.0f, nullptr, turkishRanges);
    } else {
        io.Fonts->AddFontDefault();
    }

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ProjectManager pm;
    int selectedNodeId = 0;

    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

        ImGui::Begin("RootHost", nullptr, hostFlags);

        // ── 1. PROJE AÇIK DEĞİLSE: GİRİŞ MODALI GÖSTER (PROJECT GATING) ──
        if (!pm.isProjectLoaded) {
            RenderProjectGatingModal(pm);
        } else {
            // ── 2. PROJE AÇIKSA: TAM TEŞEKKÜLLÜ IDE ÇALIŞIR ──────────────
            bool isEn = (pm.currentLanguage == AppLanguage::English);

            if (ImGui::BeginMenuBar()) {
                ImGui::TextColored(ImVec4(0, 0.85f, 1.0f, 1.0f), "[C-ALGORITHM IDE]");
                ImGui::SameLine(0, 20);
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), isEn ? "|  Project: %s" : "|  Proje: %s", pm.projectName.c_str());

                ImGui::SameLine(0, 30);
                if (ImGui::Button(isEn ? "Save Project" : "Projeyi Kaydet")) { pm.SaveProject(); }
                ImGui::SameLine();
                if (ImGui::Button(isEn ? "Close" : "Kapat")) { pm.CloseProject(); }

                ImGui::SameLine(ImGui::GetWindowWidth() - 350);
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", pm.statusMessage.c_str());
                ImGui::EndMenuBar();
            }

            float inspectorWidth = 320.0f;
            ImVec2 mainAreaSize(ImGui::GetContentRegionAvail().x - inspectorWidth - 10.0f, ImGui::GetContentRegionAvail().y);

            ImGui::BeginChild("WorkspacesContainer", mainAreaSize, false);
            if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem(isEn ? "Flow Graph Event Canvas" : "Akış Şeması Olay Grafiği")) {
                    RenderFlowGraphWorkspace(pm, selectedNodeId);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(isEn ? "Data Structure Advisor" : "Veri Yapısı Danışmanı")) {
                    RenderAdvisorWorkspace(pm);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(isEn ? "Custom Print Scene" : "Konsol Çıktı Tasarımcısı")) {
                    RenderCustomPrintScene(pm);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(isEn ? "Live C++ Code" : "Canlı C++ Kodu")) {
                    RenderCodegenWorkspace(pm);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(isEn ? "Settings" : "Ayarlar")) {
                    RenderSettingsWorkspace(pm);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            // Sağ Inspector Paneli
            ImGui::BeginChild("InspectorContainer", ImVec2(inspectorWidth, 0), true);
            RenderInspectorPanel(pm, selectedNodeId);
            ImGui::EndChild();
        }

        ImGui::End();

        // Render
        ImGui::Render();
        const float clear_color[4] = { 0.05f, 0.07f, 0.09f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // V-Sync 144 Hz
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 144;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, 
                                               featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, 
                                               &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

    switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED) return 0;
            if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
