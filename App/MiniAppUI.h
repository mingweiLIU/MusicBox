#pragma once
#include <d2d1.h>
#include <d2d1helper.h>
#include <wincodec.h>
#include <string>
#include <dwrite.h>
#include<vector>
#include "Player.h"
#include "SpectumTimer.h"
#include "SceneManager.h"



template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


//控制按钮类型
enum class ControlMode
{
    Play,//正常更新
    Pause,//暂停
    Stop,//停止
    COUNT//永远放在最后 用于确定枚举数量
};

enum class ButtonType
{
    Play,//播放跳动
    Pause,//暂停
    Stop,//停止
    Close,//关闭
    CubeMode,//纯波谱立体模式
    CubeDropMode,//纯波谱立体上方加一个小块
    WaveMode,//纯波谱
    WaveDropMode,//纯波谱并下掉
    DoubleWaveMode,//纯波谱+下掉的波谱
};

struct ControlButton
{
public:
    ControlButton(ButtonType type, int centerX, int centerY,int btnSize) {
        this->type = type;
        this->centerX = centerX;
        this->centerY = centerY;
        this->size = btnSize;
        int halfSize = btnSize / 2;
        this->rc = { centerX - halfSize,centerY - halfSize,centerX + halfSize,centerY + halfSize };
    }
    void setState(State newState) {
        this->state = newState;
    }
    BOOL isInBtn(POINT pos) {
        return PtInRect(&rc, pos);
    }
public:
    ButtonType type;
    State state;
    RECT rc;
    int centerX;
    int centerY;
    int size;
};

class MiniAppUI {
public:
    MiniAppUI(HWND parent, SceneManager& sceneManager,int width = 134, int height = 60, int cornerRadius = 10);
    ~MiniAppUI();

    bool Show();
    void Close();
    void SetBackgroundColor(D2D1_COLOR_F color);
    void SetBorderColor(D2D1_COLOR_F color, float width);
    void Resize(int width, int height);

private:
    HWND m_hwnd;
    HWND m_parent;
    SceneManager& sceneManager;

    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;//窗体渲染对象
    ID2D1SolidColorBrush* m_pBgBrush;
    ID2D1SolidColorBrush* m_pBorderBrush;
    ID2D1SolidColorBrush* m_trackBgBrush;
	ID2D1DCRenderTarget* m_pDCRenderTarget;//内容渲染对象
	IDWriteTextFormat* m_titleBarTextFormat;//在一开始就要初始化
	ID2D1SolidColorBrush* m_tileBarTextBrush;//在一开始就要初始化
    IDWriteFactory* m_pDWriteFactory;

    POINT lastMousePos;//鼠标位置
    bool isMouseTracking=false;//正在跟踪鼠标位置
    POINT actualWindowPos;  // 记录窗口实际位置
	Player* player;//播放对象
	SpectumTimer* spectumTimer;//波谱周期性更新
    std::vector<ControlButton> controlBtns;
	std::vector<SongInfo> tracks = {};

    /////////////////////////////////////////////////////////////////////
    //整个窗体
    RECT rcClient;//整个窗体矩形
    int m_cornerRadius=15;//圆角半径
    D2D1_COLOR_F m_bgColor = colorConvert(37, 48, 63, 200);
    D2D1_COLOR_F m_borderColor = colorConvert(77, 97, 109);
    D2D1_COLOR_F m_titleBarTextColor = colorConvert(220, 220, 230);
    float m_borderWidth = 2.0f;
    int m_contentMargin = 20;
    std::wstring m_titleBarText = L"播放列表";

    //控制面板
    D2D1_COLOR_F colorControlBtn = colorConvert(50, 55, 65);
    D2D1_COLOR_F colorControlBtnHover = colorConvert(60, 65, 75);
    D2D1_COLOR_F colorControlBtnPressed = colorConvert(80, 90, 100);
    D2D1_COLOR_F colorControlBtnIcon = colorConvert(255, 255, 255);
    D2D1_COLOR_F colorContolBtnPlayCicle = colorConvert(43, 98, 205);
    int m_BtnRadius = 16;
    int m_BtnCircleRadius = 24;
    int m_BtnSpacing = 20;
    SpectumMode m_displayMode = SpectumMode::Cube;
    ControlMode m_controlMode = ControlMode::Play;

    ///////////////////////////////////////////////////////////////////////////////
    //绘制类
    //整体更新
    void UpdateWindowAppearance();
    //绘制整个界面
    void DrawAll(ID2D1RenderTarget* pRenderTarget);
    //绘制整体的框架
    void DrawMiniFrame(ID2D1RenderTarget* pRenderTarget);
    //绘制控制面板
    void DrawControlBtns(ID2D1RenderTarget* pRenderTarget);
    //绘制控制面板上的各个按钮
    void DrawControlButton(ID2D1RenderTarget* pRenderTarget, ControlButton& controlBtn);
    //绘制闭合几何 这里fill表示是否是填充还是绘制线
    void DrawGeometry(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush, std::vector<D2D1_POINT_2F>& geomPoints, bool fill = true);
	//绘制sin
	void DrawSin(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush, D2D1_POINT_2F centerPoint, float width, float height, float amplitude, float frequency);
	//绘制cos
	void DrawCos(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush, D2D1_POINT_2F centerPoint, float width, float height, float amplitude, float frequency);

    //////////////////////////////////////////////////////////////////////////////
    //事件类
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void onWMMouseMove(HWND);
    //鼠标离开事件
    void onWMMouseLeave(HWND hwnd);
    //鼠标点击事件
    void onWMLButtonDown(HWND hwnd);


    //////////////////////////////////////////////////////////////////
    //工具类函数
    D2D1::ColorF colorConvert(int r, int g, int b, int a = 255);
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
	//////////////////////////////////////////////////////////////////////////
    //////////////////下面是交互函数
    // 点击切换播放展示模式
	void SwitchDisplayMode();
	//点击播放歌曲
	void PlayTrack(int index);
	//点击暂停播放
	void PausePlay();
	//重启播放
	void ResumePlay();
};
