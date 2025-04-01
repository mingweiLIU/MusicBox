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

    //�����ر���
    RECT rcClient;//������
    int m_width;//�����
    int m_height;//���߶�
    int m_cornerRadius=23;//���Բ�ǰ뾶
    Gdiplus::Color m_bgColor= Gdiplus::Color(200, 37, 48, 63);//��屳��ɫ
    Gdiplus::Color m_borderColor= Gdiplus::Color(77, 97, 109);//���߿�ɫ
    float m_borderWidth=2.0f;//���߿���
    int m_contentMargin = 20;//������ݾ������Һ͵ײ��ľ���


    //��������ر���
    RECT rcTitleBar;//����������
    RECT rcCloseBtnRect;//�����Ϲرհ�ť
    int m_titleBarHeight = 60;//����߶�
    int m_closeBtnSize = 24;//�رհ�ť��С
    Gdiplus::Color colorTitleBar = Gdiplus::Color(200, 40, 45, 55);//���ⱳ��ɫ
    COLORREF colorTitleBarHover= RGB(50, 55, 65);//�������hover��ɫ
    COLORREF colorCloseBtn = RGB(100, 110, 120);
    COLORREF colorCloseBtnHover = RGB(200, 80, 80);
    COLORREF colorTitleText = RGB(220, 220, 230);
    bool closeBtnHovered = false;

    /////////////////////////////////////////////////////////////////////////////////
    //���ܺ���


    /////////////////////////////////////////////////////////////////////////////////
    //���ƺ���
    void UpdateWindowAppearance();
    //����Բ�Ǿ���
    void DrawRoundRect(HDC hdc);
    //���Ʊ�����
    void DrawTitleBar(HDC hdc);


    /////////////////////////////////////////////////////////////////////////////////
    //�¼�����
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    //�����¼�
    void onWMPAINT(HWND hwnd, WPARAM wParam, LPARAM lParam);

    ULONG_PTR m_gdiplusToken;
};