//// RoundWindowWithPath.cpp - 使用GraphicsPath实现圆角窗体
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <objidl.h>
//#include <gdiplus.h>
//#include <memory>
//
//#pragma comment(lib, "gdiplus.lib")
//#pragma comment(lib, "user32.lib")
//
//using namespace Gdiplus;
//
//// 全局变量
//HWND g_hWnd = NULL;
//const int WINDOW_WIDTH = 400;
//const int WINDOW_HEIGHT = 300;
//const int CORNER_RADIUS = 20;
//
//// 前向声明
//LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//void OnPaint(HWND hWnd);
//void UpdateWindowRegion(HWND hWnd);
//
//// 应用程序入口点
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//{
//    // 初始化GDI+
//    GdiplusStartupInput gdiplusStartupInput;
//    ULONG_PTR gdiplusToken;
//    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//
//    // 注册窗口类
//    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
//    wcex.style = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc = WndProc;
//    wcex.hInstance = hInstance;
//    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
//    wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH); // 透明背景
//    wcex.lpszClassName = "RoundWindowWithPathClass";
//
//    if (!RegisterClassEx(&wcex))
//        return 0;
//
//    // 计算窗口位置居中
//    RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
//    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX), FALSE);
//    int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
//    int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
//
//    // 创建窗口
//    g_hWnd = CreateWindowEx(
//        WS_EX_LAYERED, // 支持透明
//        wcex.lpszClassName,
//        "GraphicsPath圆角窗体",
//        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
//        x, y,
//        rc.right - rc.left, rc.bottom - rc.top,
//        NULL, NULL, hInstance, NULL);
//
//    if (!g_hWnd)
//        return 0;
//
//    // 设置窗口为分层窗口并支持透明
//    SetLayeredWindowAttributes(g_hWnd, 0, 255, LWA_ALPHA);
//
//    // 更新窗口区域
//    UpdateWindowRegion(g_hWnd);
//
//    // 显示窗口
//    ShowWindow(g_hWnd, nCmdShow);
//    UpdateWindow(g_hWnd);
//
//    // 消息循环
//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0))
//    {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    // 关闭GDI+
//    GdiplusShutdown(gdiplusToken);
//
//    return (int)msg.wParam;
//}
//
//// 使用GraphicsPath创建圆角矩形路径
//GraphicsPath* CreateRoundRectPath(int width, int height, int radius)
//{
//    GraphicsPath* path = new GraphicsPath();
//
//    // 定义圆角矩形的各个点
//    int diameter = radius * 2;
//
//    // 左上角圆弧
//    path->AddArc(0, 0, diameter, diameter, 180, 90);
//    // 上边线
//    path->AddLine(radius, 0, width - radius, 0);
//    // 右上角圆弧
//    path->AddArc(width - diameter, 0, diameter, diameter, 270, 90);
//    // 右边线
//    path->AddLine(width, radius, width, height - radius);
//    // 右下角圆弧
//    path->AddArc(width - diameter, height - diameter, diameter, diameter, 0, 90);
//    // 下边线
//    path->AddLine(width - radius, height, radius, height);
//    // 左下角圆弧
//    path->AddArc(0, height - diameter, diameter, diameter, 90, 90);
//    // 左边线
//    path->AddLine(0, height - radius, 0, radius);
//
//    path->CloseFigure();
//    return path;
//}
//
//// 更新窗口区域
//void UpdateWindowRegion(HWND hWnd)
//{
//    // 创建GraphicsPath
//    std::unique_ptr<GraphicsPath> path(CreateRoundRectPath(WINDOW_WIDTH, WINDOW_HEIGHT, CORNER_RADIUS));
//
//    // 将路径转换为区域
//    Region region(path.get());
//
//    // 获取区域数据
//    UINT regionDataSize = 0;
//    region.GetRegionData(NULL, 0, &regionDataSize);
//
//    if (regionDataSize > 0)
//    {
//        BYTE* regionData = new BYTE[regionDataSize];
//        region.GetRegionData(regionData, regionDataSize, &regionDataSize);
//
//        // 创建HRGN
//        HRGN hRgn = ExtCreateRegion(NULL, regionDataSize, (RGNDATA*)regionData);
//        if (hRgn)
//        {
//            // 设置窗口区域
//            SetWindowRgn(hWnd, hRgn, TRUE);
//        }
//
//        delete[] regionData;
//    }
//}
//
//// 窗口过程
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    switch (message)
//    {
//    case WM_PAINT:
//        OnPaint(hWnd);
//        break;
//
//    case WM_NCHITTEST:
//    {
//        // 允许拖动窗口
//        LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
//        if (hit == HTCLIENT) hit = HTCAPTION;
//        return hit;
//    }
//
//    case WM_SIZE:
//        // 窗口大小改变时更新区域
//        UpdateWindowRegion(hWnd);
//        break;
//
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
//
//// 绘制函数
//void OnPaint(HWND hWnd)
//{
//    PAINTSTRUCT ps;
//    HDC hdc = BeginPaint(hWnd, &ps);
//
//    // 创建内存DC用于双缓冲
//    HDC hdcMem = CreateCompatibleDC(hdc);
//    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
//    SelectObject(hdcMem, hBitmap);
//
//    // 创建GDI+图形对象
//    Graphics graphics(hdcMem);
//    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
//    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
//
//    // 创建圆角矩形路径
//    std::unique_ptr<GraphicsPath> path(CreateRoundRectPath(WINDOW_WIDTH, WINDOW_HEIGHT, CORNER_RADIUS));
//
//    // 填充背景
//    SolidBrush bgBrush(Color(255, 240, 240, 240)); // 浅灰色背景
//    graphics.FillPath(&bgBrush, path.get());
//
//    // 绘制边框
//    Pen borderPen(Color(255, 100, 100, 100), 1.0f); // 灰色边框
//    graphics.DrawPath(&borderPen, path.get());
//
//    // 绘制标题
//    FontFamily fontFamily(L"Arial");
//    Font font(&fontFamily, 16, FontStyleBold, UnitPixel);
//    SolidBrush textBrush(Color(255, 50, 50, 50)); // 深灰色文字
//    graphics.DrawString(L"GraphicsPath圆角窗体", -1, &font, PointF(20, 20), &textBrush);
//
//    // 将内存DC内容复制到屏幕DC
//    BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdcMem, 0, 0, SRCCOPY);
//
//    // 清理资源
//    DeleteObject(hBitmap);
//    DeleteDC(hdcMem);
//
//    EndPaint(hWnd, &ps);
//}