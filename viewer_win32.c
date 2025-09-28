// viewer_win32.c

#define _USE_MATH_DEFINES
#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM, GET_WHEEL_DELTA_WPARAM
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vector2.h"

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)  ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)  ((int)(short)HIWORD(lp))
#endif
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wp) ((short)HIWORD(wp))
#endif

// --------------------------- Camera & Utils ----------------------------------

typedef struct {
    float scale;   // pixels per world unit
    float panX;    // additional pixel offset X
    float panY;    // additional pixel offset Y
} Camera;

static Camera g_cam = { 80.0f, 0.0f, 0.0f };
static int g_clientW = 800, g_clientH = 600;

static inline float clampf(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

static inline POINT world_to_screen(float x, float y) {
    POINT p;
    p.x = (LONG)(g_clientW * 0.5f + g_cam.panX + x * g_cam.scale);
    p.y = (LONG)(g_clientH * 0.5f + g_cam.panY - y * g_cam.scale);
    return p;
}

static inline vec2 screen_to_world(LONG sx, LONG sy) {
    float x = ((float)sx - (g_clientW * 0.5f) - g_cam.panX) / g_cam.scale;
    float y = ((g_clientH * 0.5f) + g_cam.panY - (float)sy) / g_cam.scale;
    return (vec2){ x, y };
}

static double nice_step_for_scale(double target_world_step) {
    if (target_world_step <= 0.0) return 1.0;
    double k = floor(log10(target_world_step));
    double base = pow(10.0, k);
    double frac = target_world_step / base;
    double m = (frac < 1.5) ? 1.0 : (frac < 3.0) ? 2.0 : (frac < 7.0) ? 5.0 : 10.0;
    return m * base;
}

// --------------------------- Labels (a,b,c,..., aa,ab,...) -------------------

static size_t g_label_counter = 0;

static void make_label(size_t idx, char* out, size_t outsz) {
    // bijective base-26: 1..26 -> a..z, 27 -> aa; idx приходит 0-базный
    if (outsz == 0) return;
    char tmp[32];
    size_t n = 0;
    size_t x = idx + 1;
    while (x > 0 && n < sizeof(tmp)) {
        x--;
        tmp[n++] = (char)('a' + (x % 26));
        x /= 26;
    }
    size_t m = (n < outsz - 1) ? n : (outsz - 1);
    for (size_t i = 0; i < m; ++i) out[i] = tmp[n - 1 - i];
    out[m] = '\0';
}

// --------------------------- Vector storage ----------------------------------

typedef struct {
    vec2     v;
    COLORREF color;
    char     label[8];
} VEntry;

typedef struct {
    VEntry* data;
    size_t len;
    size_t cap;
} VecList;

static VecList g_vecs = { NULL, 0, 0 };

static void veclist_reserve(VecList* v, size_t want) {
    if (want <= v->cap) return;
    size_t newCap = v->cap ? v->cap * 2 : 16;
    if (newCap < want) newCap = want;
    VEntry* nd = (VEntry*)realloc(v->data, newCap * sizeof(VEntry));
    if (!nd) return;
    v->data = nd;
    v->cap  = newCap;
}
static void veclist_push(VecList* v, vec2 value, COLORREF col) {
    if (v->len + 1 > v->cap) veclist_reserve(v, v->len + 1);
    VEntry* e = &v->data[v->len];
    e->v = value;
    e->color = col;
    make_label(g_label_counter++, e->label, sizeof(e->label));
    v->len++;
}
static void veclist_clear(VecList* v) { v->len = 0; }
static void veclist_free (VecList* v) { free(v->data); v->data = NULL; v->len = v->cap = 0; }

// ------------------------------ Drawing --------------------------------------

static void draw_grid_and_axes(HDC hdc) {
    HBRUSH bg = CreateSolidBrush(RGB(15, 16, 20));
    RECT rc = {0,0,g_clientW,g_clientH};
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);
    SetBkMode(hdc, TRANSPARENT);

    vec2 wLT = screen_to_world(0, 0);
    vec2 wRB = screen_to_world(g_clientW, g_clientH);
    double wx0 = wLT.x, wx1 = wRB.x;
    double wy0 = wRB.y, wy1 = wLT.y;
    if (wx0 > wx1) { double t=wx0; wx0=wx1; wx1=t; }
    if (wy0 > wy1) { double t=wy0; wy0=wy1; wy1=t; }

    double target_world_step = 80.0 / (double)g_cam.scale;
    double step = nice_step_for_scale(target_world_step);

    HPEN penGrid = CreatePen(PS_SOLID, 1, RGB(40, 42, 48));
    HPEN oldPen  = SelectObject(hdc, penGrid);

    double xStart = floor(wx0 / step) * step;
    for (double x = xStart; x <= wx1 + 1e-9; x += step) {
        POINT p0 = world_to_screen((float)x, (float)wy0);
        POINT p1 = world_to_screen((float)x, (float)wy1);
        MoveToEx(hdc, p0.x, p0.y, NULL);
        LineTo(hdc,  p1.x, p1.y);
    }

    double yStart = floor(wy0 / step) * step;
    for (double y = yStart; y <= wy1 + 1e-9; y += step) {
        POINT p0 = world_to_screen((float)wx0, (float)y);
        POINT p1 = world_to_screen((float)wx1, (float)y);
        MoveToEx(hdc, p0.x, p0.y, NULL);
        LineTo(hdc,  p1.x, p1.y);
    }

    HPEN penAxes = CreatePen(PS_SOLID, 2, RGB(90, 180, 255));
    SelectObject(hdc, penAxes);

    POINT x0p = world_to_screen((float)wx0, 0.0f);
    POINT x1p = world_to_screen((float)wx1, 0.0f);
    MoveToEx(hdc, x0p.x, x0p.y, NULL); LineTo(hdc, x1p.x, x1p.y);

    POINT y0p = world_to_screen(0.0f, (float)wy0);
    POINT y1p = world_to_screen(0.0f, (float)wy1);
    MoveToEx(hdc, y0p.x, y0p.y, NULL); LineTo(hdc, y1p.x, y1p.y);

    HFONT font = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");

    HFONT oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, RGB(170, 170, 170));
    char buf[64];
    int labelEvery = 2;
    for (double x = xStart; x <= wx1 + 1e-9; x += step * labelEvery) {
        POINT p = world_to_screen((float)x, 0.0f);
        snprintf(buf, sizeof(buf), "%.3g", x);
        TextOutA(hdc, p.x + 2, p.y + 2, buf, (int)strlen(buf));
    }
    for (double y = yStart; y <= wy1 + 1e-9; y += step * labelEvery) {
        POINT p = world_to_screen(0.0f, (float)y);
        snprintf(buf, sizeof(buf), "%.3g", y);
        TextOutA(hdc, p.x + 4, p.y - 16, buf, (int)strlen(buf));
    }

    // restore
    SelectObject(hdc, oldFont); DeleteObject(font);
    SelectObject(hdc, oldPen);  DeleteObject(penAxes); DeleteObject(penGrid);
}

static void draw_arrow_with_label(HDC hdc, vec2 from, const VEntry* e) {
    vec2 to = e->v;

    HPEN pen = CreatePen(PS_SOLID, 2, e->color);
    HPEN old = SelectObject(hdc, pen);
    POINT p0 = world_to_screen(from.x, from.y);
    POINT p1 = world_to_screen(to.x,   to.y);
    MoveToEx(hdc, p0.x, p0.y, NULL);
    LineTo(hdc,  p1.x, p1.y);

    float Lpx = 10.0f, Wpx = 6.0f;
    float Lw = Lpx / g_cam.scale, Ww = Wpx / g_cam.scale;

    vec2 v      = vec2_sub(&to, &from);
    float vlen2 = vec2_length2(&v);
    if (vlen2 > 1e-12f) {
        vec2 dir  = vec2_normalize(&v);
        vec2 perp = (vec2){ -dir.y, dir.x };

        vec2 tip  = to;
        vec2 dirL = vec2_mul(&dir, Lw);
        vec2 base = vec2_sub(&tip, &dirL);

        vec2 perpW = vec2_mul(&perp, Ww);
        vec2 left  = vec2_add(&base, &perpW);
        vec2 right = vec2_sub(&base, &perpW);

        POINT pl = world_to_screen(left.x,  left.y);
        POINT pr = world_to_screen(right.x, right.y);
        MoveToEx(hdc, pl.x, pl.y, NULL); LineTo(hdc, p1.x, p1.y);
        MoveToEx(hdc, pr.x, pr.y, NULL); LineTo(hdc, p1.x, p1.y);
    }

    char txt[64];
    float len = sqrtf(e->v.x * e->v.x + e->v.y * e->v.y);
    snprintf(txt, sizeof(txt), "%s  |%s|=%.3f", e->label, e->label, (double)len);

    HFONT font = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT oldFont = SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(240,240,240));
    TextOutA(hdc, p1.x + 8, p1.y - 14, txt, (int)strlen(txt));
    SelectObject(hdc, oldFont); DeleteObject(font);

    SelectObject(hdc, old);
    DeleteObject(pen);
}

static void draw_vectors(HDC hdc) {
    for (size_t i = 0; i < g_vecs.len; ++i) {
        draw_arrow_with_label(hdc, (vec2){0,0}, &g_vecs.data[i]);
    }
}

// ------------------------------ Presets --------------------------------------

typedef void (*PresetFn)(void);
typedef struct { const char* name; PresetFn fn; } PresetDesc;

// хелперы
static inline void add_vec_col(float x, float y, COLORREF c) {
    veclist_push(&g_vecs, (vec2){x,y}, c);
}
static void reset_list_and_labels(void) {
    veclist_clear(&g_vecs);
    g_label_counter = 0;
}

static void preset_empty(void) { reset_list_and_labels(); }


static void preset_basis(void) {
    reset_list_and_labels();
    add_vec_col( 2.0f, 0.0f, RGB(230,80,80));   // a
    add_vec_col( 0.0f, 2.0f, RGB(80,160,255));  // b
    add_vec_col(-2.0f, 0.0f, RGB(160,90,90));   // c
    add_vec_col( 0.0f,-2.0f, RGB(90,120,180));  // d
    add_vec_col( 1.5f, 1.5f, RGB(90,220,120));  // e
    add_vec_col(-1.5f, 1.5f, RGB(220,180,90));  // f
}

static void preset_spokes(void) {
    reset_list_and_labels();
    const int N = 16;
    const float R = 3.0f;
    for (int i = 0; i < N; ++i) {
        float a = (float)i * (float)(2.0 * M_PI / N);
        add_vec_col(cosf(a) * R, sinf(a) * R, RGB(120,210,140));
    }
}

static void preset_random(void) {
    reset_list_and_labels();
    srand((unsigned)time(NULL));
    for (int i = 0; i < 40; ++i) {
        float x = ((rand() / (float)RAND_MAX) * 10.0f) - 5.0f;
        float y = ((rand() / (float)RAND_MAX) *  6.0f) - 3.0f;
        add_vec_col(x, y, RGB(80,220,160));
    }
}

static void preset_projection(void) {
    reset_list_and_labels();
    vec2 a = (vec2){ 3.0f, 2.0f };
    vec2 b = (vec2){ 4.0f, 1.0f };
    vec2 p = vec2_project(&a, &b);
    add_vec_col(a.x, a.y, RGB(90,200,255)); // a
    add_vec_col(b.x, b.y, RGB(255,160,60)); // b
    add_vec_col(p.x, p.y, RGB(255,220,0));  // c
}

static void preset_reflection(void) {
    reset_list_and_labels();
    vec2 i = (vec2){ 3.0f,-2.0f };
    vec2 n = (vec2){ 0.0f, 1.0f };
    vec2 r = vec2_reflect(&i, &n);
    add_vec_col(i.x, i.y, RGB(90,200,255));  // a
    add_vec_col(n.x, n.y, RGB(255,160,60));  // b
    add_vec_col(r.x, r.y, RGB(255,80,200));  // c
}


static void preset_rotations(void) {
    reset_list_and_labels();
    vec2 v = (vec2){ 4.0f, 0.0f };
    for (int k = 0; k < 12; ++k) {
        float a = (float)k * (float)(2.0 * M_PI / 12.0);
        vec2 r = vec2_rotate(&v, a);
        add_vec_col(r.x, r.y, RGB(100,210,130));
    }
}

static PresetDesc g_presets[] = {
    {"Empty",                 preset_empty},
    {"Basis & Diagonals",     preset_basis},
    {"Spokes Circle",         preset_spokes},
    {"Random Vectors",        preset_random},
    {"Projection (a onto b)", preset_projection},
    {"Reflection (i about n)",preset_reflection},
    {"Rotations",             preset_rotations},
};
static const int g_preset_count = (int)(sizeof(g_presets)/sizeof(g_presets[0]));
static int g_preset_index = 0;
static const char* g_preset_name = "Empty";

static void preset_apply_index(int idx) {
    if (g_preset_count == 0) return;
    if (idx < 0) idx = (g_preset_count - 1);
    if (idx >= g_preset_count) idx = 0;
    g_preset_index = idx;
    g_preset_name = g_presets[g_preset_index].name;
    g_presets[g_preset_index].fn();
}
static void preset_next(void) { preset_apply_index(g_preset_index + 1); }
static void preset_prev(void) { preset_apply_index(g_preset_index - 1); }

// ------------------------------ Window proc ----------------------------------

static BOOL g_rightDragging = FALSE;
static POINT g_lastMouse = {0,0};

static void handle_zoom_at_cursor(short wheelDelta, int mx, int my) {
    vec2 w0 = screen_to_world(mx, my);
    float zoomFactor = (wheelDelta > 0) ? 1.1f : 1.0f / 1.1f;
    g_cam.scale = clampf(g_cam.scale * zoomFactor, 10.0f, 2000.0f);
    POINT s1 = world_to_screen(w0.x, w0.y);
    g_cam.panX += (float)(mx - s1.x);
    g_cam.panY += (float)(my - s1.y);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        preset_apply_index(0);
        return 0;

    case WM_SIZE:
        g_clientW = LOWORD(lParam);
        g_clientH = HIWORD(lParam);
        return 0;

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        vec2 w = screen_to_world(mx, my);
        veclist_push(&g_vecs, w, RGB(80,220,160));
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_RBUTTONDOWN:
        g_rightDragging = TRUE;
        g_lastMouse.x = GET_X_LPARAM(lParam);
        g_lastMouse.y = GET_Y_LPARAM(lParam);
        SetCapture(hWnd);
        return 0;

    case WM_MOUSEMOVE:
        if (g_rightDragging) {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            g_cam.panX += (float)(mx - g_lastMouse.x);
            g_cam.panY += (float)(my - g_lastMouse.y);
            g_lastMouse.x = mx;
            g_lastMouse.y = my;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;

    case WM_RBUTTONUP:
        g_rightDragging = FALSE;
        ReleaseCapture();
        return 0;

    case WM_MOUSEWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        POINT scr = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &scr);
        handle_zoom_at_cursor(delta, scr.x, scr.y);
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_KEYDOWN:
        if (wParam == VK_DELETE) {
            veclist_clear(&g_vecs);
            g_label_counter = 0;
            InvalidateRect(hWnd, NULL, FALSE);
        } else if (wParam == 'R') {
            g_cam.scale = 80.0f; g_cam.panX = 0.0f; g_cam.panY = 0.0f;
            InvalidateRect(hWnd, NULL, FALSE);
        } else if (wParam == '1') {
            preset_prev();
            InvalidateRect(hWnd, NULL, FALSE);
        } else if (wParam == '2') {
            preset_next();
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, g_clientW, g_clientH);
        HGDIOBJ oldBmp = SelectObject(memDC, bmp);

        draw_grid_and_axes(memDC);
        draw_vectors(memDC);

        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, RGB(200,200,200));
        char info[256];
        snprintf(info, sizeof(info),
                 "Preset: %s  |  1:Prev  2:Next  |  LMB:Add  RMB:Pan  Wheel:Zoom  R:Reset  Del:Clear  (Vectors: %u)",
                 g_preset_name, (unsigned)g_vecs.len);
        TextOutA(memDC, 8, 8, info, (int)strlen(info));

        BitBlt(hdc, 0, 0, g_clientW, g_clientH, memDC, 0, 0, SRCCOPY);

        // cleanup
        SelectObject(memDC, oldBmp);
        DeleteObject(bmp);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        veclist_free(&g_vecs);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// --------------------------------- WinMain -----------------------------------

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    (void)hPrev; (void)lpCmd;

    WNDCLASSA wc = {0};
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = "VecViewerWin32";

    if (!RegisterClassA(&wc)) return 1;

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT r = {0, 0, g_clientW, g_clientH};
    AdjustWindowRect(&r, style, FALSE);

    HWND hWnd = CreateWindowA(
        wc.lpszClassName, "Vector Viewer (Win32 + GDI) — Labels & Lengths",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top,
        NULL, NULL, hInstance, NULL
    );
    if (!hWnd) return 2;

    ShowWindow(hWnd, nShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
