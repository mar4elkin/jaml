#include <stdio.h>
#include <bemapiset.h>
#include <windows.h>
#include <windowsx.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(void)
{
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc = {0};
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = "VecViewerWin32";

    if (!RegisterClassA(&wc)) return 1;

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT r = {0, 0, 1000, 800};
    AdjustWindowRect(&r, style, FALSE);

    HWND hWnd = CreateWindowA(
        wc.lpszClassName, "JAML",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top,
        NULL, NULL, hInstance, NULL
    );
    if (!hWnd) return 2;

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}