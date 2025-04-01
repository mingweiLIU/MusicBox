#pragma once
#include <windows.h>
#include <windowsx.h> 
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

class RoundedWindow {
public:
    RoundedWindow(HWND parent = nullptr);
    ~RoundedWindow();

    bool Show(int width= 328, int height=492, int cornerRadius = 15);
    void Close();
    void SetBackgroundColor(Gdiplus::Color color);
    void SetBorderColor(Gdiplus::Color color, float width = 1.0f);

private:  
    HWND m_hwnd;
    HWND m_parent;

    //面板相关变量
    RECT rcClient;//面板矩形
    int m_width;//面板宽度
    int m_height;//面板高度
    int m_cornerRadius=23;//面板圆角半径
    Gdiplus::Color m_bgColor= Gdiplus::Color(200, 37, 48, 63);//面板背景色
    Gdiplus::Color m_borderColor= Gdiplus::Color(77, 97, 109);//面板边框色
    float m_borderWidth=2.0f;//面板边框宽度
    int m_contentMargin = 20;//面板内容距离左右和底部的距离


    //标题栏相关变量
    RECT rcTitleBar;//标题栏矩形
    RECT rcCloseBtnRect;//标题上关闭按钮
    int m_titleBarHeight = 60;//标题高度
    int m_closeBtnSize = 24;//关闭按钮大小
    Gdiplus::Color colorTitleBar = Gdiplus::Color(200, 40, 45, 55);//标题背景色
    COLORREF colorTitleBarHover= RGB(50, 55, 65);//标题鼠标hover颜色
    COLORREF colorCloseBtn = RGB(100, 110, 120);
    COLORREF colorCloseBtnHover = RGB(200, 80, 80);
    COLORREF colorTitleText = RGB(220, 220, 230);
    bool closeBtnHovered = false;

    /////////////////////////////////////////////////////////////////////////////////
    //功能函数


    /////////////////////////////////////////////////////////////////////////////////
    //绘制函数
    void UpdateWindowAppearance();
    //绘制圆角矩形
    void DrawRoundRect(HDC hdc);
    //绘制标题栏
    void DrawTitleBar(HDC hdc);


    /////////////////////////////////////////////////////////////////////////////////
    //事件函数
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    //绘制事件
    void onWMPAINT(HWND hwnd, WPARAM wParam, LPARAM lParam);

    ULONG_PTR m_gdiplusToken;
};