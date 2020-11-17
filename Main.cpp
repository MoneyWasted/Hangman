#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include "basewin.h"

#pragma comment(lib, "d2d1")
using namespace D2D1;

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class MainWindow : public BaseWindow<MainWindow>
{
    ID2D1Factory*           pFactory;
    ID2D1HwndRenderTarget*  pRenderTarget;
    ID2D1SolidColorBrush*   pBrush;
    ID2D1SolidColorBrush*   GroundColorBrush;
    ID2D1SolidColorBrush*   GallowColorBrush;
    ID2D1SolidColorBrush*   GrassColorBrush;
    D2D1_ELLIPSE            ellipse;

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();

public:
    MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL) {}
    PCWSTR  ClassName() const { return L"Hangman Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Recalculate drawing layout when the size of the window changes.
void MainWindow::CalculateLayout()
{
    if (pRenderTarget != NULL)
    {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        const float x = size.width / 2;
        const float y = size.height / 2;
        const float radius = min(x, y);
        ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F MainColor = D2D1::ColorF(1.0f, 1.0f, 0);
            hr = pRenderTarget->CreateSolidColorBrush(MainColor, &pBrush);

            const D2D1_COLOR_F GroundColor = D2D1::ColorF(0.4392f, 0.2824f, 0.2353f, 1.0f);
            hr = pRenderTarget->CreateSolidColorBrush(GroundColor, &GroundColorBrush);

            const D2D1_COLOR_F GallowColor = D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f);
            hr = pRenderTarget->CreateSolidColorBrush(GallowColor, &GallowColorBrush);

            const D2D1_COLOR_F GrassColor = D2D1::ColorF(0.28f, 0.72f, 0.20f, 1.0f);
            hr = pRenderTarget->CreateSolidColorBrush(GrassColor, &GrassColorBrush);

            if (SUCCEEDED(hr)) { CalculateLayout(); }
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        pRenderTarget->BeginDraw();

        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

        // Draw the ground
        pRenderTarget->DrawLine(Point2F(0, 1003), Point2F(10000, 1003), GroundColorBrush, 1000.0f, NULL);

        // Draw the grass
        pRenderTarget->DrawLine(Point2F(0, 510), Point2F(10000, 510), GrassColorBrush, 50.0f, NULL);

        // Draw the Gallows
        pRenderTarget->DrawLine(Point2F(250, 100), Point2F(50, 100), GallowColorBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(250, 100), Point2F(250, 150), GallowColorBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(50, 100), Point2F(50, 500), GallowColorBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(10, 500), Point2F(90, 500), GallowColorBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(140, 100), Point2F(50, 200), GallowColorBrush, 20.0f, NULL);

        // Draw the man
        ellipse = D2D1::Ellipse(D2D1::Point2F(250, 175), 45, 45);
        pRenderTarget->FillEllipse(ellipse, pBrush);
        pRenderTarget->DrawLine(Point2F(250, 175), Point2F(250, 350), pBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(250, 350), Point2F(200, 400), pBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(250, 350), Point2F(300, 400), pBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(250, 250), Point2F(200, 300), pBrush, 20.0f, NULL);
        pRenderTarget->DrawLine(Point2F(250, 250), Point2F(300, 300), pBrush, 20.0f, NULL);

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) { DiscardGraphicsResources(); }
        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow win;

    if (!win.Create(L"Hangman", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;  // Fail CreateWindowEx.
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}