//#pragma once
//#include <d2d1.h>
//#include <d2d1helper.h>
//#include <wincodec.h>
//#include <string>
//#include <dwrite.h>
//#include<vector>
//#include "Player.h"
//#include "SpectumTimer.h"
//
//
//
//template <class T> void SafeRelease(T** ppT)
//{
//    if (*ppT)
//    {
//        (*ppT)->Release();
//        *ppT = NULL;
//    }
//}
//
////波形模式
//enum class SpectumMode
//{
//    Cube,//纯波谱立体模式
//    Wave,//纯波谱立体上方加一个小块来模拟波形线
//    Throw,//纯波谱立体+小方块向上飞离
//    COUNT//永远放在最后 用于确定枚举数量
//};
//
//
//
//enum class ButtonType
//{
//    Previous,
//    Play,
//    Pause,
//    Next,
//    Add,
//    AddFolder,//添加文件夹
//    CubeMode,//纯波谱立体模式
//    WaveMode,//纯波谱立体上方加一个小块来模拟波形线
//    ThrowMode//纯波谱立体+小方块向上飞离
//};
//
//struct ControlButton
//{
//public:
//    ControlButton(ButtonType type, int centerX, int centerY,int btnSize) {
//        this->type = type;
//        this->centerX = centerX;
//        this->centerY = centerY;
//        this->size = btnSize;
//        int halfSize = btnSize / 2;
//        this->rc = { centerX - halfSize,centerY - halfSize,centerX + halfSize,centerY + halfSize };
//    }
//    void setState(State newState) {
//        this->state = newState;
//    }
//    BOOL isInBtn(POINT pos) {
//        return PtInRect(&rc, pos);
//    }
//public:
//    ButtonType type;
//    State state;
//    RECT rc;
//    int centerX;
//    int centerY;
//    int size;
//};
//
//class AppUI {
//public:
//    AppUI(HWND parent,int width = 328, int height = 492, int cornerRadius = 15);
//    ~AppUI();
//
//    bool Show();
//    void Close();
//    void SetBackgroundColor(D2D1_COLOR_F color);
//    void SetBorderColor(D2D1_COLOR_F color, float width);
//    void Resize(int width, int height);
//
//private:
//    HWND m_hwnd;
//    HWND m_parent;
//
//    ID2D1Factory* m_pD2DFactory;
//    ID2D1HwndRenderTarget* m_pRenderTarget;//窗体渲染对象
//    ID2D1SolidColorBrush* m_pBgBrush;
//    ID2D1SolidColorBrush* m_pBorderBrush;
//    ID2D1DCRenderTarget* m_pDCRenderTarget;//内容渲染对象
//    IDWriteFactory* m_pDWriteFactory;
//
//    POINT lastMousePos;//鼠标位置
//    bool isMouseTracking=false;//正在跟踪鼠标位置
//    POINT actualWindowPos;  // 记录窗口实际位置
//	Player* player;//播放对象
//	SpectumTimer* spectumTimer;//波谱周期性更新
//
//    std::vector<SongInfo> tracks = {
//		/*SongInfo(L"Halo", L"洛特·凯斯特纳",L""),
//		SongInfo(L"Young for you", L"迦拉",L""),
//		SongInfo(L"Bohemian Rhapsody", L"皇后乐队",L""),
//		SongInfo(L"Hotel California", L"老鹰乐队",L""),
//		SongInfo(L"Imagine", L"约翰·列侬",L""),
//		SongInfo(L"Halo", L"洛特·凯斯特纳",L""),
//		SongInfo(L"Young for you", L"迦拉",L""),
//		SongInfo(L"Bohemian Rhapsody", L"皇后乐队",L""),
//		SongInfo(L"Hotel California", L"老鹰乐队",L""),
//		SongInfo(L"Imagine", L"约翰·列侬",L""),
//		SongInfo(L"Halo", L"洛特·凯斯特纳",L""),
//		SongInfo(L"Young for you", L"迦拉",L""),
//		SongInfo(L"Bohemian Rhapsody", L"皇后乐队",L""),
//		SongInfo(L"Hotel California", L"老鹰乐队",L""),
//		SongInfo(L"Imagine", L"约翰·列侬",L""),
//		SongInfo(L"Halo", L"洛特·凯斯特纳",L""),
//		SongInfo(L"Young for you", L"迦拉",L""),
//		SongInfo(L"Bohemian Rhapsody", L"皇后乐队",L""),
//		SongInfo(L"Hotel California", L"老鹰乐队",L""),
//		SongInfo(L"Imagine", L"约翰·列侬",L"")*/
//    };
//    std::vector<ControlButton> controlBtns;
//
//    /////////////////////////////////////////////////////////////////////
//    //整个窗体
//    RECT rcClient;//整个窗体矩形
//    int m_cornerRadius=15;//圆角半径
//    D2D1_COLOR_F m_bgColor = colorConvert(37, 48, 63, 200);
//    D2D1_COLOR_F m_borderColor = colorConvert(77, 97, 109);
//    D2D1_COLOR_F m_titleBarTextColor = colorConvert(220, 220, 230);
//    float m_borderWidth = 2.0f;
//    int m_contentMargin = 20;
//    std::wstring m_titleBarText = L"播放列表";
//
//    //标题栏
//    RECT rcTitleBar;
//    RECT rcCloseBtn;
//    int m_titleBarHeight = 60;
//    int m_closeBtnHeight = 24;
//    D2D1_COLOR_F colorTitleBar = colorConvert(40, 45, 55, 200);
//    D2D1_COLOR_F colorTitleBarHover = colorConvert(50, 55, 65);
//    D2D1_COLOR_F  colorCloseBtnX = colorConvert(255, 255, 255);
//    D2D1_COLOR_F colorCloseBtnHoverBG = colorConvert(143, 43, 43);
//    D2D1_COLOR_F colorTitleText = colorConvert(220, 220, 230);
//    D2D1_COLOR_F colorTitleBarBottomLine = colorConvert(100, 43, 43);
//    bool closeBtnHovered = false;
//    IDWriteTextFormat* m_titleBarTextFormat;//在一开始就要初始化
//    ID2D1SolidColorBrush* m_tileBarTextBrush;//在一开始就要初始化
//
//    //滚动栏
//    RECT rcScrollTrack;
//    RECT rcScrollThrumb;
//    int m_scrollWidth = 10;
//    int m_scrollThrumbMinHeight = 30;
//    int m_scrollThrumbHeight;
//    int m_scrollPos=0;
//    int m_scrollMax=0;
//    D2D1_COLOR_F colorScrollBg = colorConvert(50, 55, 65);
//    D2D1_COLOR_F colorScrollThumb = colorConvert(80, 85, 95);
//    D2D1_COLOR_F colorScrollThumbHover = colorConvert(100, 105, 115);
//    State m_scrollThumbState = State::NONE;
//    int m_scrollDragStartY = 0;
//    int m_scrollInitialPos = 0;
//
//    //列表
//    RECT rcList;
//    int m_trackHeight = 55;
//    D2D1_COLOR_F colorTrackBg = colorConvert(40, 45, 55);
//    D2D1_COLOR_F colorTrackHover = colorConvert(50, 55, 65);
//    D2D1_COLOR_F colorTrackSelected = colorConvert(70, 80, 90);
//    D2D1_COLOR_F colorTrackText = colorConvert(220, 220, 230);
//    D2D1_COLOR_F colorTrackTextSelected = colorConvert(255, 255, 255);
//    D2D1_COLOR_F colorTrackSecondaryText = colorConvert(120, 130, 140);
//    D2D1_COLOR_F colorTrackSecondaryTextSelected = colorConvert(180, 180, 180);
//    D2D1_COLOR_F colorTrackSeparator = colorConvert(60, 65, 75);
//    float m_trackTextSize = 14.0f;
//    ID2D1SolidColorBrush* m_trackBgBrush;
//    ID2D1SolidColorBrush* m_trackSeparetorBrush;
//    IDWriteTextFormat* m_trackTextFormat;//在一开始就要初始化
//    ID2D1SolidColorBrush* m_trackTextBrush;//在一开始就要初始化
//    IDWriteTextFormat* m_trackSecondaryTextFormat;//在一开始就要初始化
//    int m_playingIndex = -1;
//    bool m_isPlaying = false;
//
//
//    //控制面板
//    RECT rcControlPanel;
//    int m_controlPanelHeigh = 100;
//    D2D1_COLOR_F colorControlPanelBg = colorConvert(40, 45, 55, 0);
//    D2D1_COLOR_F colorControlBtn = colorConvert(50, 55, 65);
//    D2D1_COLOR_F colorControlBtnHover = colorConvert(60, 65, 75);
//    D2D1_COLOR_F colorControlBtnPressed = colorConvert(80, 90, 100);
//    D2D1_COLOR_F colorControlBtnIcon = colorConvert(255, 255, 255);
//    D2D1_COLOR_F colorContolBtnPlayCicle = colorConvert(43, 98, 205);
//    int m_BtnSize = 32;
//    int m_BtnSpacing = 20;
//    SpectumMode m_displayMode = SpectumMode::Throw;
//
//	//绘制时间线
//	RECT rcTimeLine;
//	RECT rcTimeLineBtn;
//	int m_timeLineHeight = 6;
//	int m_timeLineWidth = 300;
//	D2D1_COLOR_F colorTimeLineProgressed = colorConvert(235, 107, 115);
//	D2D1_COLOR_F colorTimeLineBg = colorConvert(218, 220, 223);
//    D2D1_COLOR_F colorTimeLineProgressedHover = colorConvert(242, 58, 69);
//	int m_timeLineBtnRadius = (m_timeLineHeight + 3);
//	D2D1_COLOR_F colorTimeLineBtn = colorConvert(180, 180, 180);
//	D2D1_COLOR_F colorTimeLineBtnHover = colorConvert(255, 255, 255);
//	D2D1_COLOR_F colorTimeLineBtnPressed = colorConvert(255, 255, 255); 
//	D2D1_COLOR_F colorTimeLineText = colorConvert(166, 170, 179); 
//	float m_playingPercent = 0.3f;
//	float m_trackLength = 0.0f;//歌曲播放时长 秒为单位
//	State m_timeLineState = State::NONE;
//    State m_timeLineBtnState = State::NONE;
//	int m_timeLineBtnPressedX = 0;
//	IDWriteTextFormat* m_timeLineTextFormat;//在一开始就要初始化
//
//    ///////////////////////////////////////////////////////////////////////////////
//    //绘制类
//    //整体更新
//    void UpdateWindowAppearance();
//    //绘制整个界面
//    void DrawAll(ID2D1RenderTarget* pRenderTarget);
//    //绘制圆角框架
//    void DrawRoundedFrame(ID2D1RenderTarget* pRenderTarget);
//    //绘制顶部为圆角的矩形
//    void DrawTopRoundedRect(ID2D1RenderTarget* pRenderTarget,const D2D1_RECT_F& rect,float topCornerRadius,ID2D1Brush* pFillBrush);
//    //绘制标题栏
//    void DrawTitleBar(ID2D1RenderTarget* pRenderTarget);
//    //绘制所有列表
//    void DrawTracks(ID2D1RenderTarget* pRenderTarget);
//    //绘制一个Track
//    void DrawOneTrack(ID2D1RenderTarget* pRenderTarget,SongInfo& songInfo,const RECT& rect);
//    //绘制滚动条
//    void DrawScrollBar(ID2D1RenderTarget* pRenderTarget);
//    //绘制控制面板
//    void DrawControlPanel(ID2D1RenderTarget* pRenderTarget);
//    //绘制底部为圆角的矩形
//    void DrawBottomRoundedRect(ID2D1RenderTarget* pRenderTarget, const D2D1_RECT_F& rect, float bottomCornerRadius, ID2D1Brush* pFillBrush);
//    //绘制控制面板上的各个按钮
//    void DrawControlButton(ID2D1RenderTarget* pRenderTarget, ControlButton& controlBtn);
//    //绘制闭合几何 这里fill表示是否是填充还是绘制线
//    void DrawGeometry(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush, std::vector<D2D1_POINT_2F>& geomPoints, bool fill = true);
//    //绘制进度条
//	void DrawTimeline(ID2D1RenderTarget* pRenderTarget);
//	//强制重绘
//	void forceRedrew();
//
//
//    //////////////////////////////////////////////////////////////////////////////
//    //事件类
//    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//    void onWMMouseMove(HWND);
//    void onMouseMove_TittleBar(HWND hwnd);
//    //鼠标在列表和滚动条上移动
//    void onMouseMove_ListAndScroll(HWND hwnd);
//    //鼠标在列表上
//    void mouseMove_List(HWND hwnd);
//    //鼠标在控制面板上
//	void mouseMove_ControlPanel(HWND hwnd);
//	//鼠标在时间进度条上移动
//	void onMouseMove_TimeLine(HWND hwnd);
//    //鼠标离开事件
//    void onWMMouseLeave(HWND hwnd);
//    //鼠标点击事件
//    void onWMLButtonDown(HWND hwnd);
//    //鼠标点击控制按钮
//    void lbuttonDownControlBtn(HWND hwnd);
//    //鼠标双击下播放track
//    void onDblClickTrack(int clickIndex);
//    //双击事件
//    void onWMLBUTTONDBLCLK(HWND hwnd);
//
//
//    //////////////////////////////////////////////////////////////////
//    //工具类函数
//    D2D1::ColorF colorConvert(int r, int g, int b, int a = 255);
//    HRESULT CreateDeviceResources();
//    void DiscardDeviceResources();
//    void UpdateScrollInfo();
//    void UpdateThumbPosition();
//    //滑到指定位置
//    void ScrollTo(int newPos);
//    //计算根据时长（单位秒)计算时间字符串 如果超过小时 则为h:m:s 没有则为m:s 如01:03:12/02:03
//    std::wstring CalcuTimeLineStr(int timeSecond);
//    
//	//////////////////////////////////////////////////////////////////////////
//    //////////////////下面是交互函数
//    // 点击切换播放展示模式
//	void SwitchDisplayMode();
//	//点击播放歌曲
//	void PlayTrack(int index);
//	//点击暂停播放
//	void PausePlay();
//	//重启播放
//	void ResumePlay();
//	//播放上一首
//	void PlayPrevious();
//	//播放下一首
//	void PlayNext();
//	//从文件夹添加音乐
//	void AddMusicFiles();
//};
