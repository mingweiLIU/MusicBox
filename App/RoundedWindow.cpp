#include "RoundedWindow.h"
#include <string>

RoundedWindow::RoundedWindow(HWND parent)
    : m_hwnd(nullptr), m_parent(parent), m_cornerRadius(15),
    m_bgColor(50, 30, 30, 30),  // 半透明深灰
    m_borderColor(255, 70, 130, 200),  // 蓝色边框
    m_borderWidth(1.5f) {}

RoundedWindow::~RoundedWindow() {
    Close();
    Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

// 创建并显示圆角窗口
bool RoundedWindow::Show(int width, int height, int cornerRadius) {
    m_cornerRadius = cornerRadius;

    // 初始化 GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    // 注册窗口类
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = RoundedWindow::WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "RoundedWindowClass";
    if (!RegisterClassEx(&wc)) return false;

    this->m_width = width; this->m_height = height;

    // 创建分层窗口（透明背景+阴影）
    m_hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        "RoundedWindowClass",
        "Rounded Window",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        m_parent,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );
    if (!m_hwnd) return false;
    GetClientRect(m_hwnd, &rcClient);

    // 初始绘制
    UpdateWindowAppearance();
    ShowWindow(m_hwnd, SW_SHOW);
    return true;
}

// 关闭窗口
void RoundedWindow::Close() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

// 设置背景色（ARGB）
void RoundedWindow::SetBackgroundColor(Gdiplus::Color color) {
    m_bgColor = color;
    if (m_hwnd) UpdateWindowAppearance();
}

// 设置边框颜色和宽度
void RoundedWindow::SetBorderColor(Gdiplus::Color color, float width) {
    m_borderColor = color;
    m_borderWidth = width;
    if (m_hwnd) UpdateWindowAppearance();
}

// 窗口消息处理
LRESULT CALLBACK RoundedWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        RoundedWindow* pThis = reinterpret_cast<RoundedWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    RoundedWindow* pThis = reinterpret_cast<RoundedWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return pThis ? pThis->HandleMessage(hwnd, msg, wParam, lParam) : DefWindowProc(hwnd, msg, wParam, lParam);
}

// 消息处理
LRESULT RoundedWindow::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }

        case WM_LBUTTONDOWN: {
            // 拖动窗口
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            break;
        }

        case WM_PAINT: {
            onWMPAINT(hwnd, wParam, lParam);
            break;
        }

        case WM_NCHITTEST: {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd, &pt);

            //如果在标题栏中 就让其能拖动
            if (PtInRect(&rcTitleBar, pt)) {
                return HTCAPTION;
            }
            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//绘制事件
void RoundedWindow::onWMPAINT(HWND hwnd, WPARAM wParam, LPARAM lParam) { 
    UpdateWindowAppearance();
}

// 绘制圆角矩形（带边框）
void RoundedWindow::DrawRoundRect(HDC hdc) {
    // 使用 GDI+ 绘制
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.Clear(Gdiplus::Color(0, 0, 0, 0)); // 透明背景
    Gdiplus::GraphicsPath path;
    path.AddArc(rcClient.left, rcClient.top, m_cornerRadius, m_cornerRadius, 180, 90); // 左上
    path.AddArc(rcClient.right - rcClient.left - m_cornerRadius, rcClient.top, m_cornerRadius, m_cornerRadius, 270, 90); // 右上
    path.AddArc(rcClient.right - rcClient.left - m_cornerRadius, rcClient.bottom - rcClient.top - m_cornerRadius, m_cornerRadius, m_cornerRadius, 0, 90); // 右下
    path.AddArc(rcClient.left, rcClient.bottom - rcClient.top - m_cornerRadius, m_cornerRadius, m_cornerRadius, 90, 90); // 左下
    path.CloseFigure();

    // 填充背景
    Gdiplus::SolidBrush bgBrush(m_bgColor);
    graphics.FillPath(&bgBrush, &path);

    // 绘制边框
    Gdiplus::GraphicsPath pathBorder;
    pathBorder.AddArc(rcClient.left, rcClient.top + (int)m_borderWidth / 2.0, m_cornerRadius, m_cornerRadius, 180, 90); // 左上
    pathBorder.AddArc(rcClient.right - rcClient.left - m_cornerRadius-(int) m_borderWidth/2.0, rcClient.top, m_cornerRadius, m_cornerRadius, 270, 90); // 右上
    pathBorder.AddArc(rcClient.right - rcClient.left - m_cornerRadius - (int)m_borderWidth / 2.0, rcClient.bottom - rcClient.top - m_cornerRadius - (int)m_borderWidth / 2.0, m_cornerRadius, m_cornerRadius, 0, 90); // 右下
    pathBorder.AddArc(rcClient.left, rcClient.bottom - rcClient.top - m_cornerRadius- (int)m_borderWidth / 2.0, m_cornerRadius, m_cornerRadius, 90, 90); // 左下
    pathBorder.CloseFigure();
    Gdiplus::Pen borderPen(m_borderColor, m_borderWidth);
    graphics.DrawPath(&borderPen, &pathBorder);
}
// 更新窗口外观（透明+圆角）
void RoundedWindow::UpdateWindowAppearance() {    
    // 创建内存 DC
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcScreen, rcClient.right, rcClient.bottom);
    SelectObject(hdcMem, hbmMem);

    DrawRoundRect(hdcMem);
    DrawTitleBar(hdcMem);

    // 更新分层窗口
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    POINT ptSrc = { 0, 0 };
    SIZE size = { rcClient.right, rcClient.bottom };
    POINT ptDst = { rcClient.left, rcClient.top };
    UpdateLayeredWindow(m_hwnd, nullptr, &ptDst, &size, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    // 清理资源
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}


//绘制标题栏
void RoundedWindow::DrawTitleBar(HDC hdc) {
    HFONT hTitleFont= CreateFont(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "SimSun");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);

    rcTitleBar = {
        0,0,rcClient.right,m_titleBarHeight
    };

    //绘制
    Gdiplus::GraphicsPath path;
    path.AddArc(rcClient.left, rcClient.top, m_cornerRadius, m_cornerRadius, 180, 90); // 左上
    path.AddArc(rcClient.right - rcClient.left - m_cornerRadius, rcClient.top, m_cornerRadius, m_cornerRadius, 270, 90); // 右上
    path.AddLine(rcTitleBar.right, rcTitleBar.bottom, rcTitleBar.left, rcTitleBar.bottom); // 右下
    path.CloseFigure();

    // 填充背景
    Gdiplus::SolidBrush bgBrush(colorTitleBar);
    // 使用 GDI+ 绘制
    Gdiplus::Graphics graphics(hdc);
    graphics.FillPath(&bgBrush, &path);

    //写标题
    SetTextColor(hdc, colorTitleText);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, L"播放列表", -1, &rcTitleBar, DT_VCENTER | DT_SINGLELINE | DT_CENTER);

    RECT rcCloseBtnRect = {
        rcTitleBar.right - m_closeBtnSize - 10,
        (m_titleBarHeight - m_closeBtnSize) / 2,
        rcTitleBar.right - 10,
        (m_titleBarHeight + m_closeBtnSize) / 2
    };

    // 使用GDI+绘制更平滑的圆形关闭按钮
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    // 更协调的颜色方案
    Gdiplus::Color btnBgColor = closeBtnHovered ? colorCloseBtnHover : colorCloseBtn;
    Gdiplus::Color btnXColor = Gdiplus::Color(255, 255, 255);

    // 绘制圆形背景
    Gdiplus::SolidBrush btnBrush(btnBgColor);
    graphics.FillEllipse(&btnBrush, rcCloseBtnRect.left, rcCloseBtnRect.top,
        rcCloseBtnRect.right - rcCloseBtnRect.left, rcCloseBtnRect.bottom - rcCloseBtnRect.top);

    // 绘制X图标
    Gdiplus::Pen xPen(btnXColor, 2.0f);
    int centerX = (rcCloseBtnRect.left + rcCloseBtnRect.right) / 2;
    int centerY = (rcCloseBtnRect.top + rcCloseBtnRect.bottom) / 2;
    int crossSize = 6;

    graphics.DrawLine(&xPen,
        centerX - crossSize, centerY - crossSize,
        centerX + crossSize, centerY + crossSize);
    graphics.DrawLine(&xPen,
        centerX + crossSize, centerY - crossSize,
        centerX - crossSize, centerY + crossSize);

    // 恢复资源
    SelectObject(hdc, hOldFont);
}

