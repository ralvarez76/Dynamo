
#include "stdafx.h"
#include "Bloodstone.h"
#include "Utilities.h"
#include "NodeGeometries.h"
#include "Resources\resource.h"

#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace Dynamo::Bloodstone;
using namespace Autodesk::DesignScript::Interfaces;

LRESULT _stdcall LocalWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Visualizer::WndProc(hWnd, msg, wParam, lParam);
}

System::IntPtr Visualizer::Create(System::IntPtr hWndParent, int width, int height)
{
    if (Visualizer::mVisualizer == nullptr)
    {
        auto hParent = ((HWND) hWndParent.ToPointer());
        Visualizer::mVisualizer = gcnew Visualizer();
        Visualizer::mVisualizer->Initialize(hParent, width, height);
    }

    return System::IntPtr(Visualizer::mVisualizer->GetWindowHandle());
}

void Visualizer::Destroy(void)
{
    if (Visualizer::mVisualizer != nullptr) {
        Visualizer::mVisualizer->Uninitialize();
        Visualizer::mVisualizer = nullptr;
    }
}

Visualizer^ Visualizer::CurrentInstance(void)
{
    return Visualizer::mVisualizer;
}

LRESULT Visualizer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (Visualizer::mVisualizer == nullptr)
        return ::DefWindowProc(hWnd, msg, wParam, lParam);

    return Visualizer::mVisualizer->ProcessMessage(hWnd, msg, wParam, lParam);
}

void Visualizer::ShowWindow(bool show)
{
    if (::IsWindow(this->mhWndVisualizer)) {
        auto cmd = show ? SW_SHOW : SW_HIDE;
        ::ShowWindow(this->mhWndVisualizer, cmd);
    }
}

void Visualizer::RequestFrameUpdate(void)
{
    ::InvalidateRect(this->mhWndVisualizer, nullptr, true); // Update window.
}

HWND Visualizer::GetWindowHandle(void)
{
    return this->mhWndVisualizer;
}

Scene^ Visualizer::GetScene(void)
{
    return this->mpScene;
}

IGraphicsContext* Visualizer::GetGraphicsContext(void)
{
    return this->mpGraphicsContext;
}

Visualizer::Visualizer() : 
    mhWndVisualizer(nullptr),
    mpScene(nullptr),
    mpGraphicsContext(nullptr)
{
}

void Visualizer::Initialize(HWND hWndParent, int width, int height)
{
    if (mhWndVisualizer != nullptr) {
        auto message = L"'Visualizer::Initialize' cannot be called twice.";
        throw gcnew System::InvalidOperationException(gcnew String(message));
    }

    WNDCLASSEX windowClass = { 0 };

    // The reason we create our own window class is that we need CS_OWNDC style 
    // to be specified for the window we are creating (OpenGL creation needs it).
    // 
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = LocalWndProc;
    windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = ::CreateSolidBrush(RGB(100, 149, 237));
    windowClass.lpszClassName = L"VisualizerClass";
    RegisterClassEx(&windowClass);

    mhWndVisualizer = CreateWindowEx(0, windowClass.lpszClassName, nullptr,
        WS_CHILD | WS_VISIBLE, 0, 0, width, height, hWndParent, nullptr, nullptr, 0);

    // Initialize graphics context for rendering.
    auto contextType = IGraphicsContext::ContextType::OpenGL;
    mpGraphicsContext = IGraphicsContext::Create(contextType);
    mpGraphicsContext->Initialize(mhWndVisualizer);

    mpScene = gcnew Scene(this);
    mpScene->Initialize(width, height);
}

void Visualizer::Uninitialize(void)
{
    if (this->mpScene != nullptr) {
        this->mpScene->Destroy();
        delete this->mpScene;
        this->mpScene = nullptr;
    }

    if (this->mpGraphicsContext != nullptr) {
        this->mpGraphicsContext->Uninitialize();
        this->mpGraphicsContext = nullptr;
    }

    if (this->mhWndVisualizer != nullptr) {
        ::DestroyWindow(this->mhWndVisualizer);
        this->mhWndVisualizer = nullptr;
    }
}

LRESULT Visualizer::ProcessMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto x = GET_X_LPARAM(lParam);
    auto y = GET_Y_LPARAM(lParam);

    auto pCamera = mpGraphicsContext->GetDefaultCamera();
    auto pTrackBall = pCamera->GetTrackBall();

    switch (msg)
    {
    case WM_LBUTTONDOWN:
        SetCapture(this->mhWndVisualizer);
        pTrackBall->MousePressed(x, y);
        break;

    case WM_LBUTTONUP:
        pTrackBall->MouseReleased(x, y);
        ::ReleaseCapture();
        break;

    case WM_MOUSEMOVE:
        if ((wParam & MK_LBUTTON) == 0)
            return 0L; // Mouse button isn't pressed.

        pTrackBall->MouseMoved(x, y);
        break;
    }

    RequestFrameUpdate(); // Update window.
    return 0L; // Message processed.
}

LRESULT Visualizer::ProcessMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC deviceContext = BeginPaint(hWnd, &ps);
            {
                mpGraphicsContext->BeginRenderFrame(deviceContext);
                mpScene->RenderScene();
                mpGraphicsContext->EndRenderFrame(deviceContext);
            }
            EndPaint(hWnd, &ps);
            return 0L;
        }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        return ProcessMouseMessage(msg, wParam, lParam);

    case WM_ERASEBKGND:
        return 0L; // Avoid erasing background to flickering during sizing.

    case WM_SIZE:
        {
            if (mpGraphicsContext != nullptr)
            {
                auto pCamera = mpGraphicsContext->GetDefaultCamera();
                if (pCamera != nullptr) {
                    pCamera->ResizeViewport(LOWORD(lParam), HIWORD(lParam));
                    return 0L; // Message processed.
                }
            }
            break;
        }
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}