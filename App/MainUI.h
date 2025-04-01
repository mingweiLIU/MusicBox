#pragma  once
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <vector>
#include <string>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <gdiplus.h>
#include <versionhelpers.h>

#include "Player.h"
#include "SpectumTimer.h"

//#pragma execution_character_set("utf-8") 

using namespace Gdiplus;
class MainUI {
public:
	MainUI(HWND hwndParent);
	~MainUI();
	void Show();
	static LRESULT CALLBACK PanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:

	//波形模式
	enum class SpectumMode
	{
		Cube,//纯波谱立体模式
		Wave,//纯波谱立体上方加一个小块来模拟波形线
		Throw,//纯波谱立体+小方块向上飞离
		COUNT//永远放在最后 用于确定枚举数量
	};

	enum class ButtonType {
		Previous,
		Play,
		Pause,
		Next,
		Add,//添加歌曲
		AddFolder,//添加文件夹
		CubeMode,//纯波谱立体模式
		WaveMode,//纯波谱立体上方加一个小块来模拟波形线
		ThrowMode//纯波谱立体+小方块向上飞离
	};

	// Add these member variables to track button states
	struct ButtonState {
		bool hovered = false;
		bool pressed = false;
	};

	struct TrackState {
		bool isHovered = false;
	};

	HWND _hwndPanel;
	HWND _hwndParent;
	POINT _dragStartPoint;
	bool _isDragging = false;
	bool _isMouseTracking = false;
	int _scrollPos = 0;
	int _scrollMax = 0;
	bool _scrollDragging = false;
	int _scrollThumbHeight = 0;
	RECT _scrollTrackRect;
	RECT _scrollThumbRect;
	POINT _lastMousePos;
	bool _scrollThumbHovered = false;
	int _scrollDragStartY = 0;
	int _scrollThumbStartY = 0;
	const int TRACK_HEIGHT = 50;
	RECT _rcClient;
	int _selectedTrackIndex = -1;
	bool _titleBarHovered = false;
	bool _closeBtnHovered = false;
	const int TITLE_BAR_HEIGHT = 40;
	const int CLOSE_BTN_SIZE = 24;
	const int PANEL_RADIUS = 16;
	const int BORDER_SIZE = 1;
	const int CONTROL_PANEL_HEIGHT = 100;
	RECT _controlPanelRect;
	bool _playBtnHovered = false;
	bool _prevBtnHovered = false;
	bool _nextBtnHovered = false;
	bool _addBtnHovered = false;
	bool _modeBtnHovered = false;
	bool _isPlaying = false;
	SpectumMode _displayMode = SpectumMode::Throw;//定义波谱显示模式

	// 颜色定义
	COLORREF _colorBackground = RGB(30, 35, 45);
	COLORREF _colorTrackDefault = RGB(40, 45, 55);
	COLORREF _colorTrackHover = RGB(50, 55, 65);
	COLORREF _colorTrackSelected = RGB(70, 80, 90);
	COLORREF _colorText = RGB(220, 220, 230);
	COLORREF _colorSecondaryText = RGB(120, 130, 140);
	COLORREF _colorSeparator = RGB(60, 65, 75);
	COLORREF _colorScrollTrack = RGB(50, 55, 65);
	COLORREF _colorScrollThumb = RGB(80, 85, 95);
	COLORREF _colorScrollThumbHover = RGB(100, 105, 115);
	COLORREF _colorTitleBar = RGB(40, 45, 55);
	COLORREF _colorTitleBarHover = RGB(50, 55, 65);
	COLORREF _colorCloseBtn = RGB(100, 110, 120);
	COLORREF _colorCloseBtnHover = RGB(200, 80, 80);
	COLORREF _colorTitleText = RGB(220, 220, 230);
	COLORREF _colorBorderActive = RGB(80, 85, 95);
	COLORREF _colorBorderInactive = RGB(60, 65, 75);
	COLORREF _colorControlPanel = RGB(40, 45, 55);
	COLORREF _colorBtnNormal = RGB(50, 55, 65);
	COLORREF _colorBtnHover = RGB(60, 65, 75);
	// Add a new color for pressed state
	COLORREF _colorBtnPressed = RGB(80, 90, 100);
	// Variables to store the current track and playlist control
	int _currentTrackIndex = -1;
	int _totalTracks = 0;
	std::wstring _currentFolder;

	std::vector<SongInfo> _tracks = {
		//SongInfo(L"Halo", L"洛特·凯斯特纳",L""),
		//SongInfo(L"Young for you", L"迦拉",L""),
		//SongInfo(L"Bohemian Rhapsody", L"皇后乐队",L""),
		//SongInfo(L"Hotel California", L"老鹰乐队",L""),
		//SongInfo(L"Imagine", L"约翰·列侬",L"")
	};
	//std::vector<std::pair<std::string, std::string>> _tracks = {
	//	{"Halo", "洛特·凯斯特纳"},
	//	{"Young for you", "迦拉"},
	//	{"Bohemian Rhapsody", "皇后乐队"},
	//	{"Hotel California", "老鹰乐队"},
	//	{"Imagine", "约翰·列侬"},
	//	{"Young for you", "迦拉"},
	//	{"Bohemian Rhapsody", "皇后乐队"},
	//	{"Bohemian Rhapsody", "皇后乐队"},
	//	{"Hotel California", "老鹰乐队"},
	//	{"Imagine", "约翰·列侬"},
	//	{"Young for you", "迦拉"},
	//	{"Bohemian Rhapsody", "皇后乐队"},
	//};
	

	ButtonState _prevBtnState;
	ButtonState _playBtnState;
	ButtonState _nextBtnState;
	ButtonState _addBtnState;
	ButtonState _modeBtnState;

	std::vector<TrackState> _trackStates;

	// 预创建的GDI资源
	HDC _hdcMem = NULL;
	HBITMAP _hbmMem = NULL;
	HFONT _hFont = NULL;
	HFONT _hTitleFont = NULL;

	ULONG_PTR _gdiplusToken;

	Player* player;//播放对象
	SpectumTimer* spectumTimer;//波谱周期性更新

	//////////////////////////////////////////////////////////////////////////
	////////////////////下面是工具函数
	std::wstring UTF8ToUnicode(const std::string& utf8str);
	//初始化GDI+
	void CreateGDIPlusResources();
	//释放GDI+资源
	void ReleaseGDIPlusResources();
	//启用DWM模式
	void EnableModernWindowStyle(HWND hwnd);
	//初始化歌曲状态
	void InitializeTrackStates();

	//////////////////////////////////////////////////////////////////////////
	//////////////////下面是绘制内容函数
	//绘制3D立体边框
	void Draw3DBorder(HDC hdc);
	//更新滑动条状态
	void UpdateScrollInfo();
	//更新滑块位置
	void UpdateThumbPosition();
	//滑到指定位置
	void ScrollTo(int newPos);
	//绘制滑动条
	void DrawScrollBar(HDC hdc);
	//绘制歌曲列表中的一个
	void DrawTrack(HDC hdc, const RECT& rect, SongInfo& track,const TrackState& state, bool isSelected);
	//绘制标题栏
	void DrawTitleBar(HDC hdc);
	//绘制控制面板
	void DrawControlPanel(HDC hdc);
	//绘制控制按钮
	void DrawControlButton(Graphics& graphics, int x, int centerY, int size, ButtonType type, bool hovered, bool pressed);
	//绘制进度条
	void DrawTimeLine(HDC hdc, int centerLineY);

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
	//播放上一首
	void PlayPrevious();
	//播放下一首
	void PlayNext();
	//从文件夹添加音乐
	void AddMusicFiles();
	//直接点击歌曲列表的一行
	void OnTrackClick(int trackIndex);

	//////////////////////////////////////////////////////////////////////////
	/////下面是事件处理函数
	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//绘制事件
	void onWMPAINT(HWND hwnd,  WPARAM wParam, LPARAM lParam);
	//鼠标移动事件
	void onWMMouseMove(HWND hwnd);
	//鼠标在标题栏上
	void onMouseMove_TittleBar(HWND hwnd);
	//鼠标在播放控制面板上
	void onMouseMove_ControlPanel(HWND hwnd);
	//鼠标在列表范围上含滑块
	void onMouseMove_ListAndScroll(HWND hwnd);
	//鼠标在列表上
	void mouseMove_List(HWND hwnd);
	//鼠标离开事件
	void onWMMouseLeave(HWND hwnd);
	//鼠标左键按下事件
	void onWMLButtonDown(HWND hwnd);
	//鼠标在控制面板上按下
	void leftButtonDown_ControlPanel(HWND hwnd);
};