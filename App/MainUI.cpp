#include "MainUI.h"
#include <filesystem>
#include <bass.h>
#include "Player.h"

MainUI::MainUI(HWND hwndParent) : _hwndParent(hwndParent) {
	// 不再需要在这里初始化GDI+，因为已经在WinMain中初始化

	 // Initialize button states
	_prevBtnState = { false, false };
	_playBtnState = { false, false };
	_nextBtnState = { false, false };
	_addBtnState = { false, false };

	// Initialize track state
	_currentTrackIndex = -1;
	_totalTracks = _tracks.size();

	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = MainUI::PanelProc;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(_colorBackground);
	wcex.lpszClassName = "ModernPlaylistPanel";
	RegisterClassEx(&wcex);

	_hwndPanel = CreateWindowEx(
		0,
		"ModernPlaylistPanel",
        "播放列表",
		WS_POPUP | WS_VISIBLE,
		800, 200,
		370, 520,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		this
	);

	EnableModernWindowStyle(_hwndPanel);
	GetClientRect(_hwndPanel, &_rcClient);

	//启动监听
	player = new Player(_hwndPanel, _tracks);
	spectumTimer = new SpectumTimer(player);
	spectumTimer->Start();
}

MainUI::~MainUI()
{
	ReleaseGDIPlusResources();
	if (spectumTimer) {
		spectumTimer->Stop();
		delete spectumTimer;
	}
	if (player) delete player;
}

void MainUI::Show()
{
	ShowWindow(_hwndPanel, SW_SHOW);
	UpdateWindow(_hwndPanel);
	UpdateScrollInfo();

	// 强制立即完整重绘
	InvalidateRect(_hwndPanel, NULL, TRUE);
	UpdateWindow(_hwndPanel);
}

LRESULT CALLBACK MainUI::PanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MainUI* pThis = reinterpret_cast<MainUI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (msg == WM_NCCREATE) {
		pThis = reinterpret_cast<MainUI*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	return pThis ? pThis->HandleMessage(hwnd, msg, wParam, lParam) : DefWindowProc(hwnd, msg, wParam, lParam);
}

std::wstring MainUI::UTF8ToUnicode(const std::string& utf8str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(utf8str);
}

void MainUI::CreateGDIPlusResources()
{
	// 预创建字体
	_hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "SimSun");

	_hTitleFont = CreateFont(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "SimSun");

	// 预创建内存DC
	HDC hdc = GetDC(_hwndPanel);
	_hdcMem = CreateCompatibleDC(hdc);
	_hbmMem = CreateCompatibleBitmap(hdc, 1000, 1000); // 足够大的尺寸
	SelectObject(_hdcMem, _hbmMem);
	ReleaseDC(_hwndPanel, hdc);
}

void MainUI::ReleaseGDIPlusResources()
{
	if (_hFont) DeleteObject(_hFont);
	if (_hTitleFont) DeleteObject(_hTitleFont);
	if (_hbmMem) DeleteObject(_hbmMem);
	if (_hdcMem) DeleteDC(_hdcMem);
}

void MainUI::EnableModernWindowStyle(HWND hwnd)
{
	// 强制启用DWM圆角效果
	DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
	DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));

	// 启用深色模式
	BOOL useImmersiveDarkMode = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode));

	// 设置现代背景
	//DWM_WINDOW_CORNER_PREFERENCE backdrop = DWMWCP_ROUND;
	//DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));

	// 设置边框颜色
	COLORREF borderColor = GetActiveWindow() == hwnd ? _colorBorderActive : _colorBorderInactive;
	DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));

	// 启用非客户区渲染
	const DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
	DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));

	// 设置阴影颜色（可选）
	COLORREF shadowColor = RGB(50, 50, 50); // 深灰色阴影
	DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &shadowColor, sizeof(shadowColor));

	// 扩展窗口框架到客户区
	// MARGINS margins = { 0, 0, TITLE_BAR_HEIGHT, 0 };
	//DwmExtendFrameIntoClientArea(hwnd, &margins);
}

void MainUI::InitializeTrackStates()
{
	_trackStates.resize(_tracks.size());
}

void MainUI::Draw3DBorder(HDC hdc) {
	// 亮色上/左边框（模拟光照）
	HPEN hLightPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hLightPen);
	MoveToEx(hdc, _rcClient.left, _rcClient.bottom - 1, NULL);
	LineTo(hdc, _rcClient.left, _rcClient.top);
	LineTo(hdc, _rcClient.right - 1, _rcClient.top);

	// 暗色下/右边框（模拟阴影）
	HPEN hDarkPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	SelectObject(hdc, hDarkPen);
	MoveToEx(hdc, _rcClient.left, _rcClient.bottom - 1, NULL);
	LineTo(hdc, _rcClient.right - 1, _rcClient.bottom - 1);
	LineTo(hdc, _rcClient.right - 1, _rcClient.top);

	// 恢复并清理
	SelectObject(hdc, hOldPen);
	DeleteObject(hLightPen);
	DeleteObject(hDarkPen);
}

void MainUI::UpdateScrollInfo()
{
	GetClientRect(_hwndPanel, &_rcClient);

	// 计算控制面板区域
	_controlPanelRect = {
		0,
		_rcClient.bottom - CONTROL_PANEL_HEIGHT,
		_rcClient.right,
		_rcClient.bottom
	};

	int contentHeight = _tracks.size() * TRACK_HEIGHT;
	_scrollMax = max(0, contentHeight - (_rcClient.bottom - TITLE_BAR_HEIGHT - CONTROL_PANEL_HEIGHT));
	_scrollPos = min(_scrollPos, _scrollMax);

	float visibleRatio = (float)(_rcClient.bottom - TITLE_BAR_HEIGHT - CONTROL_PANEL_HEIGHT) / contentHeight;
	_scrollThumbHeight = max(30, (int)((_rcClient.bottom - TITLE_BAR_HEIGHT - CONTROL_PANEL_HEIGHT) * visibleRatio));

	_scrollTrackRect = _rcClient;
	_scrollTrackRect.left = _rcClient.right - 10;
	_scrollTrackRect.top += TITLE_BAR_HEIGHT;
	_scrollTrackRect.bottom -= CONTROL_PANEL_HEIGHT;

	UpdateThumbPosition();
}

void MainUI::UpdateThumbPosition()
{
	if (_scrollMax > 0) {
		float thumbPosRatio = (float)_scrollPos / _scrollMax;
		int thumbY = (int)(thumbPosRatio * (_scrollTrackRect.bottom - _scrollThumbHeight - TITLE_BAR_HEIGHT));

		_scrollThumbRect = {
			_scrollTrackRect.left,
			_scrollTrackRect.top + thumbY,
			_scrollTrackRect.right,
			_scrollTrackRect.top + thumbY + _scrollThumbHeight
		};
	}
	else {
		_scrollThumbRect = { 0, 0, 0, 0 };
	}
}

void MainUI::ScrollTo(int newPos)
{
	newPos = max(0, min(_scrollMax, newPos));
	if (newPos != _scrollPos) {
		_scrollPos = newPos;
		UpdateThumbPosition();

		RECT rcClient;
		GetClientRect(_hwndPanel, &rcClient);
		RECT rcInvalidate = rcClient;
		rcInvalidate.top = TITLE_BAR_HEIGHT;
		rcInvalidate.bottom -= CONTROL_PANEL_HEIGHT;
		InvalidateRect(_hwndPanel, &rcInvalidate, FALSE);
		UpdateWindow(_hwndPanel);
	}
}

void MainUI::DrawScrollBar(HDC hdc)
{
	HBRUSH hTrackBrush = CreateSolidBrush(_colorScrollTrack);
	FillRect(hdc, &_scrollTrackRect, hTrackBrush);
	DeleteObject(hTrackBrush);

	if (_scrollMax > 0) {
		COLORREF thumbColor = _scrollThumbHovered ? _colorScrollThumbHover : _colorScrollThumb;
		HBRUSH hThumbBrush = CreateSolidBrush(thumbColor);
		FillRect(hdc, &_scrollThumbRect, hThumbBrush);
		DeleteObject(hThumbBrush);
	}
}

void MainUI::DrawTrack(HDC hdc, const RECT& rect, SongInfo& track, const TrackState& state, bool isSelected)
{	
	// 使用预创建的字体
	HFONT hOldFont = (HFONT)SelectObject(hdc, _hFont);

	COLORREF bgColor = isSelected ? _colorTrackSelected :
		state.isHovered ? _colorTrackHover : _colorTrackDefault;

	HBRUSH hBrush = CreateSolidBrush(bgColor);
	FillRect(hdc, &rect, hBrush);
	DeleteObject(hBrush);

	SetTextColor(hdc, isSelected ? RGB(255, 255, 255) : _colorText);
	SetBkMode(hdc, TRANSPARENT);

	RECT titleRect = rect;
	titleRect.left += 10;
	titleRect.right -= 20;
	std::wstring wTitle = track.name;
	DrawTextW(hdc, wTitle.c_str(), -1, &titleRect, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	SetTextColor(hdc, isSelected ? RGB(180, 180, 180) : _colorSecondaryText);
	RECT artistRect = rect;
	artistRect.right -= 20;
	std::wstring wArtist = track.artist;
	DrawTextW(hdc, wArtist.c_str(), -1, &artistRect, DT_VCENTER | DT_SINGLELINE | DT_RIGHT | DT_END_ELLIPSIS);

	//绘制分割线
	HPEN hPen = CreatePen(PS_SOLID, 1, _colorSeparator);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rect.left, rect.bottom - 1, NULL);
	LineTo(hdc, rect.right - 10, rect.bottom - 1);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	SelectObject(hdc, hOldFont);
}

void MainUI::DrawTitleBar(HDC hdc)
{
	// 使用预创建的标题字体
	HFONT hOldFont = (HFONT)SelectObject(hdc, _hTitleFont);

	RECT titleBarRect = {
		0,
		0,
		_rcClient.right,
		TITLE_BAR_HEIGHT
	};

	// 标题栏背景（DWM模式下使用透明背景）
	HBRUSH hBrush = CreateSolidBrush(_titleBarHovered ? _colorTitleBarHover : _colorTitleBar);
	FillRect(hdc, &titleBarRect, hBrush);
	DeleteObject(hBrush);

	SetTextColor(hdc, _colorTitleText);
	SetBkMode(hdc, TRANSPARENT);
    std::string wTitle = "播放列表";
	DrawText(hdc, wTitle.c_str(), -1, &titleBarRect, DT_VCENTER | DT_SINGLELINE | DT_CENTER);

	RECT closeBtnRect = {
		_rcClient.right - CLOSE_BTN_SIZE - 10,
		(TITLE_BAR_HEIGHT - CLOSE_BTN_SIZE) / 2,
		_rcClient.right - 10,
		(TITLE_BAR_HEIGHT + CLOSE_BTN_SIZE) / 2
	};

	// 使用GDI+绘制更平滑的圆形关闭按钮
	Graphics graphics(hdc);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	// 更协调的颜色方案
	Color btnBgColor = _closeBtnHovered ? Color(232, 17, 35) : Color(80, 80, 80);
	Color btnXColor = Color(255, 255, 255);

	// 绘制圆形背景
	SolidBrush btnBrush(btnBgColor);
	graphics.FillEllipse(&btnBrush, closeBtnRect.left, closeBtnRect.top,
		closeBtnRect.right - closeBtnRect.left, closeBtnRect.bottom - closeBtnRect.top);

	// 绘制X图标
	Pen xPen(btnXColor, 2.0f);
	int centerX = (closeBtnRect.left + closeBtnRect.right) / 2;
	int centerY = (closeBtnRect.top + closeBtnRect.bottom) / 2;
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

void MainUI::DrawControlPanel(HDC hdc)
{
	// Panel background
	HBRUSH hPanelBrush = CreateSolidBrush(_colorControlPanel);
	FillRect(hdc, &_controlPanelRect, hPanelBrush);
	DeleteObject(hPanelBrush);

	// Separator line
	HPEN hLinePen = CreatePen(PS_SOLID, 1, _colorSeparator);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hLinePen);
	MoveToEx(hdc, _controlPanelRect.left, _controlPanelRect.top, NULL);
	LineTo(hdc, _controlPanelRect.right, _controlPanelRect.top);
	SelectObject(hdc, hOldPen);
	DeleteObject(hLinePen);

	// Use GDI+ for smoother rendering
	Graphics graphics(hdc);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	// Calculate button positions
	int btnSize = 32;
	int btnSpacing = 20;
	int totalWidth = btnSize * 5 + btnSpacing * 4;
	int startX = (_controlPanelRect.right - totalWidth) / 2;
	int centerY = (_controlPanelRect.top + _controlPanelRect.bottom) / 2;

	// Draw the actual buttons with their current states
	ButtonType displayModeBtnType = ButtonType::CubeMode;
	if (_displayMode == SpectumMode::Wave) {
		displayModeBtnType = ButtonType::WaveMode;
	}
	else if (_displayMode==SpectumMode::Throw)
	{
		displayModeBtnType = ButtonType::ThrowMode;
	}
	DrawControlButton(graphics, startX, centerY, btnSize,
		displayModeBtnType, _modeBtnState.hovered, _modeBtnState.pressed);

	DrawControlButton(graphics, startX + btnSize + btnSpacing, centerY, btnSize, ButtonType::Previous,
		_prevBtnState.hovered, _prevBtnState.pressed);

	DrawControlButton(graphics, startX + (btnSize + btnSpacing)*2, centerY, btnSize,
		_isPlaying ? ButtonType::Pause : ButtonType::Play,
		_playBtnState.hovered, _playBtnState.pressed);

	DrawControlButton(graphics, startX + (btnSize + btnSpacing) * 3, centerY, btnSize,
		ButtonType::Next, _nextBtnState.hovered, _nextBtnState.pressed);

	DrawControlButton(graphics, startX + (btnSize + btnSpacing) * 4, centerY, btnSize,
		ButtonType::Add, _addBtnState.hovered, _addBtnState.pressed);

	

	// If we have a current track playing, display its info
	if (_currentTrackIndex >= 0 && _currentTrackIndex < _tracks.size()) {
		HFONT hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "SimSun");
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		SetTextColor(hdc, _colorText);
		SetBkMode(hdc, TRANSPARENT);

		RECT textRect = {
			10,
			_controlPanelRect.top + 5,
			_controlPanelRect.right - 10,
			_controlPanelRect.top + 20
		};

		// Display current track name and artist
		std::wstring trackInfo = _tracks[_currentTrackIndex].name + L" - " +_tracks[_currentTrackIndex].artist;
		DrawTextW(hdc, trackInfo.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
	}
}

void MainUI::DrawControlButton(Graphics& graphics, int x, int centerY, int size, ButtonType type, bool hovered, bool pressed)
{
	// Apply scaling effect when hovered
	float scale = hovered ? 1.1f : 1.0f;
	int adjustedSize = (int)(size * scale);

	// Center the enlarged button
	int adjustedX = x - (adjustedSize - size) / 2;
	int adjustedY = centerY - adjustedSize / 2;

	// Button background color based on state
	Color btnColor;
	if (pressed) {
		btnColor = Color(_colorBtnPressed);
	}
	else if (hovered) {
		btnColor = Color(_colorBtnHover);
	}
	else {
		btnColor = Color(_colorBtnNormal);
	}

	// Draw button background with anti-aliasing
	SolidBrush btnBrush(btnColor);
	graphics.FillEllipse(&btnBrush, adjustedX, adjustedY, adjustedSize, adjustedSize);

	// Calculate icon dimensions (slightly smaller than button)
	float iconScale = 0.5f;
	float iconSize = adjustedSize * iconScale;
	float iconX = adjustedX + (adjustedSize - iconSize) / 2;
	float iconY = adjustedY + (adjustedSize - iconSize) / 2;

	// Draw the appropriate icon based on button type
	SolidBrush iconBrush(Color(255, 255, 255));
	SolidBrush iconRedBrush(Color(43, 98, 205));
	Pen iconPen(Color(255, 255, 255), 2.0f * scale);

	switch (type) {
	case ButtonType::Previous: {
		// Previous track icon (triangle pointing left)
		//绘制一个竖条在左侧
		float barWidth = iconSize / 6;
		RectF rect1(iconX, iconY, barWidth, iconSize);
		graphics.FillRectangle(&iconBrush, rect1);

		PointF points[3] = {
			PointF(iconX + iconSize, iconY),
			PointF(iconX , iconY + iconSize / 2),
			PointF(iconX + iconSize , iconY + iconSize)
		};
		graphics.FillPolygon(&iconBrush, points, 3);
		break;
	}
	case ButtonType::Play: {
		// Play icon (triangle pointing right)
		//在外面画一个大的圈 并用其他颜色标识
		graphics.FillEllipse(&iconRedBrush, (int)(iconX - iconSize *0.75), (int)(iconY - iconSize * 0.75), 2.5 * iconSize, 2.5 * iconSize);

		float offset = 1;
		PointF points[3] = {
			PointF(iconX+offset, iconY-offset),
			PointF(iconX + iconSize + offset, iconY + iconSize / 2 - offset),
			PointF(iconX + offset, iconY + iconSize - offset)
		};
		graphics.FillPolygon(&iconBrush, points, 3);

		break;
	}
	case ButtonType::Pause: {
		// Pause icon (two vertical rectangles)
		//在外面画一个大的圈 并用其他颜色标识
		graphics.FillEllipse(&iconRedBrush, (int)(iconX - iconSize * 0.75), (int)(iconY - iconSize * 0.75), 2.5 * iconSize, 2.5 * iconSize);

		float barWidth = iconSize / 3;
		float spacing = iconSize / 6;

		RectF rect1(iconX+1, iconY, barWidth, iconSize);
		RectF rect2(iconX+1 + barWidth + spacing, iconY, barWidth, iconSize);

		graphics.FillRectangle(&iconBrush, rect1);
		graphics.FillRectangle(&iconBrush, rect2);
		break;
	}
	case ButtonType::Next: {
		// Next track icon (triangle pointing right)
		//绘制一个竖条在右侧
		float barWidth = iconSize / 6;
		RectF rect1(iconX + iconSize-barWidth, iconY, barWidth, iconSize);
		graphics.FillRectangle(&iconBrush, rect1);
		PointF points[3] = {
			PointF(iconX, iconY),
			PointF(iconX + iconSize, iconY + iconSize / 2),
			PointF(iconX, iconY + iconSize)
		};
		graphics.FillPolygon(&iconBrush, points, 3);
		break;
	}
	case ButtonType::Add: {
		// Add icon (plus sign)
		//float middle = iconX + iconSize / 2;
		//float quarterSize = iconSize / 4;

		//graphics.DrawLine(&iconPen,

		//	iconX + quarterSize, iconY + iconSize / 2,
		//	iconX + iconSize - quarterSize, iconY + iconSize / 2);

		//// Vertical line
		//graphics.DrawLine(&iconPen,
		//	middle, iconY + quarterSize,
		//	middle, iconY + iconSize - quarterSize);
		float offset = iconSize / 8;
		float space = iconSize / 4;//横线和田字的间隔
		graphics.DrawLine(&iconPen, iconX + offset, iconY, iconX + iconSize - offset, iconY);

		float leftUpX = iconX; float leftUpY = iconY + space;
		float leftDownX = iconX + offset; float leftDownY = iconY + iconSize;
		float rightUpX = iconX + iconSize; float rightUpY = iconY + space;
		float righDownX = iconX + iconSize - offset; float rightDownY = iconY + iconSize;
		Point points[] = {
			Point(leftUpX,leftUpY),
			Point(leftDownX,leftDownY),
			Point(righDownX,rightDownY),
			Point(rightUpX,rightUpY),
			Point(leftUpX,leftUpY)
		};
		graphics.DrawLines(&iconPen, points, 5);

		//再绘制一个十字
		float crossRadius = 4;
		graphics.DrawLine(&iconPen, iconX + iconSize / 2 - crossRadius, iconY + (space + iconSize) / 2 , iconX + iconSize / 2 + crossRadius, iconY + (space + iconSize) / 2 );
		graphics.DrawLine(&iconPen, iconX + iconSize / 2, iconY + (space + iconSize) / 2- crossRadius, iconX + iconSize / 2 , iconY + (space + iconSize) / 2+ crossRadius);

		break;
	}
	case ButtonType::AddFolder: {
		//绘制一个田字形并在上面加一横
		//先画横
		float offset = iconSize / 8;
		float space = iconSize / 8;//横线和田字的间隔
		graphics.DrawLine(&iconPen, iconX + offset, iconY, iconX + iconSize - offset, iconY);

		float leftUpX = iconX; float leftUpY = iconY + space;
		float leftDownX = iconX + offset; float leftDownY = iconY + iconSize;
		float rightUpX = iconX + iconSize; float rightUpY = iconY + space;
		float righDownX = iconX + iconSize - offset; float rightDownY = iconY + iconSize;
		Point points[] = {
			Point(leftUpX,leftUpY),
			Point(leftDownX,leftDownY),
			Point(righDownX,rightDownY),
			Point(rightUpX,rightUpY),
			Point(leftUpX,leftUpY)
		};
		graphics.DrawLines(&iconPen, points, 5);

		break;

	}
	case ButtonType::CubeMode: {
		// 纯立体模式 绘制三个竖线吧
		float barWidth = iconSize / 8;
		float spacing = iconSize / 6;

		RectF rect1(iconX, iconY + 2, barWidth, iconSize - 2);
		RectF rect2(iconX + barWidth + spacing, iconY, barWidth, iconSize);
		RectF rect3(iconX + (barWidth + spacing) * 2, iconY + 4, barWidth, iconSize - 4);
		RectF rect4(iconX + (barWidth + spacing) * 3, iconY + 2, barWidth, iconSize - 2);

		graphics.FillRectangle(&iconBrush, rect1);
		graphics.FillRectangle(&iconBrush, rect2);
		graphics.FillRectangle(&iconBrush, rect3);
		graphics.FillRectangle(&iconBrush, rect4);
		break;
	}
	case ButtonType::WaveMode: {
		float barWidth = iconSize / 8;
		float spacing = iconSize / 6;

		RectF rect1(iconX, iconY + 6, barWidth, iconSize - 6);
		RectF rect2(iconX + barWidth + spacing, iconY + 8, barWidth, iconSize - 8);
		RectF rect3(iconX + (barWidth + spacing) * 2, iconY+4, barWidth, iconSize-4);
		RectF rect4(iconX + (barWidth + spacing) * 3, iconY + 9, barWidth, iconSize - 9);

		graphics.FillRectangle(&iconBrush, rect1);
		graphics.FillRectangle(&iconBrush, rect2);
		graphics.FillRectangle(&iconBrush, rect3);
		graphics.FillRectangle(&iconBrush, rect4);

		Point points[] = {
			Point(iconX, iconY + 2),
			Point(iconX + barWidth + spacing, iconY + 4),
			Point(iconX + (barWidth + spacing) * 2, iconY),
			Point(iconX + (barWidth + spacing) * 3+2, iconY + 5)
		};
		graphics.DrawLines(&iconPen, points, 4);
		break;
	}
	case ButtonType::ThrowMode: {
		float barWidth = iconSize / 8;
		float spacing = iconSize / 6;

		RectF rect1(iconX, iconY , barWidth, iconSize - 2);
		RectF rect2(iconX + barWidth + spacing, iconY+4 , barWidth, iconSize-2 );
		RectF rect3(iconX + (barWidth + spacing) * 2, iconY , barWidth, iconSize - 2);
		RectF rect4(iconX + (barWidth + spacing) * 3, iconY + 4, barWidth, iconSize - 2);

		graphics.FillRectangle(&iconBrush, rect1);
		graphics.FillRectangle(&iconBrush, rect2);
		graphics.FillRectangle(&iconBrush, rect3);
		graphics.FillRectangle(&iconBrush, rect4);
		break;
	}
	}

	// Add subtle glow effect when hovered
	if (hovered) {
		GraphicsPath path;
		path.AddEllipse(adjustedX, adjustedY, adjustedSize, adjustedSize);

		// Create a subtle glow effect
		PathGradientBrush glowBrush(&path);
		Color glowColor(50, 255, 255, 255);  // Semi-transparent white
		glowBrush.SetCenterColor(glowColor);

		Color surroundColors[] = { Color(0, 255, 255, 255) };  // Transparent at edges
		int count = 1;
		glowBrush.SetSurroundColors(surroundColors, &count);

		// Extended glow radius
		float expandedSize = adjustedSize * 1.2f;
		float expandOffset = (expandedSize - adjustedSize) / 2;
		graphics.FillEllipse(&glowBrush,
			adjustedX - expandOffset,
			adjustedY - expandOffset,
			expandedSize,
			expandedSize);
	}
}

void MainUI::DrawTimeLine(HDC hdc,int centerLineY) {
	HFONT hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "SimSun");
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	SetTextColor(hdc, _colorText);
	SetBkMode(hdc, TRANSPARENT);

	RECT textRect = {
		10,
		centerLineY + 5,
		_controlPanelRect.right - 10,
		centerLineY + 20
	};

	// Display current track name and artist
	std::wstring trackInfo = _tracks[_currentTrackIndex].name + L" - " + _tracks[_currentTrackIndex].artist;
	DrawTextW(hdc, trackInfo.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void MainUI::SwitchDisplayMode()
{
	// 转换为整数进行计算
	int next = static_cast<int>(_displayMode) + 1;

	// 如果超过枚举值数量则循环回到第一个
	if (next >= static_cast<int>(SpectumMode::COUNT)) {
		next = 0;
	}

	// 转换回枚举类型
	_displayMode = static_cast<SpectumMode>(next);
}

void MainUI::PlayTrack(int index)
{
	if (index >= 0 && index < _tracks.size()) {
		_selectedTrackIndex = index;
		_currentTrackIndex = index;
		_isPlaying = true;

		// Force redraw of track list and control panel
		RECT rcTracks = { 0, TITLE_BAR_HEIGHT, _rcClient.right, _controlPanelRect.top };
		InvalidateRect(_hwndPanel, &rcTracks, FALSE);
		InvalidateRect(_hwndPanel, &_controlPanelRect, FALSE);

		// Here you would add actual audio playback code
		// For this example, we'll just update the UI
		//player->PlaySong(_currentTrackIndex);
	}
}



void MainUI::PausePlay()
{
	_isPlaying = false;
	InvalidateRect(_hwndPanel, &_controlPanelRect, FALSE);
	// Here you would pause the actual audio playback
	player->PauseSong();
}

void MainUI::ResumePlay()
{
	_isPlaying = true;
	InvalidateRect(_hwndPanel, &_controlPanelRect, FALSE);
	// Here you would resume the actual audio playback
	if (player->isPlaying()) {
		player->PauseResumSong();
	}
	else {
		_currentTrackIndex = 0;
		player->PlaySong(_currentTrackIndex);
	}
}

void MainUI::PlayPrevious()
{
	if (_currentTrackIndex > 0) {
		PlayTrack(_currentTrackIndex - 1);
	}
	else if (!_tracks.empty()) {
		// 直接播放最后一首
		PlayTrack(_tracks.size() - 1);
	}
	player->PlaySong(_currentTrackIndex);
}

void MainUI::PlayNext()
{
	if (_currentTrackIndex < _tracks.size() - 1) {
		PlayTrack(_currentTrackIndex + 1);
	}
	else if (!_tracks.empty()) {
		// 播放第一首
		PlayTrack(0);
	}
	player->PlaySong(_currentTrackIndex);
}

void MainUI::AddMusicFiles()
{
	// Open file dialog to select music files
	OPENFILENAMEW ofn;
	wchar_t szFile[4028] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = _hwndPanel;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Music Files\0*.mp3;*.wav;*.flac;*.m4a\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (GetOpenFileNameW(&ofn)) {
		// For this example, we'll just add a dummy track
		// In a real application, you would parse the files and add them
		// 第一个字符串是目录路径
		const wchar_t* p = ofn.lpstrFile;
		std::wstring directory = p;
		p += directory.length() + 1;  // 移动到第一个文件名

		// 如果只选了一个文件，lpstrFile 直接是完整路径
		if (*p == 0) {
			std::filesystem::path selectdFilePath(directory);
			;
			_tracks.push_back(SongInfo(selectdFilePath.stem().wstring(),L"未知",directory));
		}
		else {
			// 遍历所有选中的文件
			while (*p) {
				std::wstring filename = p;
				std::wstring oneSelectdFilePathStr = directory + L"\\" + filename;
				std::filesystem::path selectdFilePath(oneSelectdFilePathStr);
				_tracks.push_back(SongInfo(selectdFilePath.stem().wstring(), L"未知", oneSelectdFilePathStr));
				p += filename.length() + 1;  // 移动到下一个文件名
			}
		}


		_trackStates.resize(_tracks.size());

		// Update scroll information since we added new tracks
		_scrollMax = max(0, _tracks.size() * TRACK_HEIGHT - (_rcClient.bottom - TITLE_BAR_HEIGHT - CONTROL_PANEL_HEIGHT));
		UpdateScrollInfo();

		// Force redraw
		InvalidateRect(_hwndPanel, NULL, FALSE);
	}
}

void MainUI::OnTrackClick(int trackIndex)
{
	if (trackIndex < 0 || trackIndex >= _tracks.size()) return;

	int oldSelection = _selectedTrackIndex;
	_selectedTrackIndex = trackIndex;
	_currentTrackIndex = trackIndex;
	_isPlaying = true;  // Start playing when a track is clicked

	player->PlaySong(_currentTrackIndex);

	RECT rcClient;
	GetClientRect(_hwndPanel, &rcClient);

	if (oldSelection != -1) {
		RECT oldRect = {
			0,
			TITLE_BAR_HEIGHT + oldSelection * TRACK_HEIGHT - _scrollPos,
			rcClient.right - 10,
			TITLE_BAR_HEIGHT + (oldSelection + 1) * TRACK_HEIGHT - _scrollPos
		};
		if (oldRect.bottom > TITLE_BAR_HEIGHT && oldRect.top < rcClient.bottom - CONTROL_PANEL_HEIGHT) {
			InvalidateRect(_hwndPanel, &oldRect, FALSE);
		}
	}

	RECT newRect = {
		0,
		TITLE_BAR_HEIGHT + trackIndex * TRACK_HEIGHT - _scrollPos,
		rcClient.right - 10,
		TITLE_BAR_HEIGHT + (trackIndex + 1) * TRACK_HEIGHT - _scrollPos
	};
	if (newRect.bottom > TITLE_BAR_HEIGHT && newRect.top < rcClient.bottom - CONTROL_PANEL_HEIGHT) {
		InvalidateRect(_hwndPanel, &newRect, FALSE);
	}

	// Also update the control panel to show play state
	InvalidateRect(_hwndPanel, &_controlPanelRect, FALSE);	
}

LRESULT MainUI::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_CREATE: {
			InitializeTrackStates();
			CreateGDIPlusResources();  // 预创建资源
			UpdateScrollInfo();

			// 立即触发首次绘制
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
			return 0;
		}

		case WM_SIZE: {
			UpdateScrollInfo();
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		}

		case WM_ACTIVATE: {
			// 更新活动状态下的边框颜色
			COLORREF borderColor = LOWORD(wParam) != WA_INACTIVE ? _colorBorderActive : _colorBorderInactive;
			DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		}

		case WM_PAINT: {
			onWMPAINT(hwnd,wParam,lParam);
		}
		case WM_MOUSEMOVE: {
			_lastMousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			onWMMouseMove(hwnd);
			return 0;
		}

		case WM_MOUSELEAVE: {
			onWMMouseLeave(hwnd);
			return 0;
		}

		case WM_LBUTTONDOWN: {
			_lastMousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			onWMLButtonDown(hwnd);
			return 0;
		}

		case WM_LBUTTONUP: {
			if (_prevBtnState.pressed || _playBtnState.pressed ||
				_nextBtnState.pressed || _addBtnState.pressed) {
				_prevBtnState.pressed = false;
				_playBtnState.pressed = false;
				_nextBtnState.pressed = false;
				_addBtnState.pressed = false;
				InvalidateRect(hwnd, &_controlPanelRect, FALSE);
			}

			if (_isDragging || _scrollDragging) {
				_isDragging = false;
				_scrollDragging = false;
				ReleaseCapture();
			}
			return 0;
		}

		case WM_MOUSEWHEEL: {
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			ScrollTo(_scrollPos - delta / 3);
			return 0;
		}

		case WM_NCHITTEST: {
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ScreenToClient(hwnd, &pt);

			RECT closeBtnRect = {
				_rcClient.right - CLOSE_BTN_SIZE - 10,
				(TITLE_BAR_HEIGHT - CLOSE_BTN_SIZE) / 2,
				_rcClient.right - 10,
				(TITLE_BAR_HEIGHT + CLOSE_BTN_SIZE) / 2
			};
			if (PtInRect(&closeBtnRect, pt)) {
				return HTCLIENT;
			}

			if (pt.y <= TITLE_BAR_HEIGHT) {
				return HTCAPTION;
			}

			if (PtInRect(&_scrollTrackRect, pt)) {
				return HTCLIENT;
			}

			int clickedIndex = (pt.y - TITLE_BAR_HEIGHT + _scrollPos) / TRACK_HEIGHT;
			if (clickedIndex >= 0 && clickedIndex < (int)_tracks.size()) {
				return HTCLIENT;
			}

			return HTCLIENT;
		}

		case WM_CLOSE: {

			DestroyWindow(hwnd);
			return 0;
		}

		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);	
}

void MainUI::onWMPAINT(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	// 双缓冲绘图
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, _rcClient.right, _rcClient.bottom);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

	// 清空背景（DWM会处理圆角和透明效果）
	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hdcMem, &_rcClient, hBrush);
	DeleteObject(hBrush);

	//绘制边框
	Draw3DBorder(hdcMem);

	// 绘制标题栏
	DrawTitleBar(hdcMem);

	// 绘制音轨列表
	HRGN hClipRgn = CreateRectRgn(0, TITLE_BAR_HEIGHT, _rcClient.right - 10, _controlPanelRect.top);
	SelectClipRgn(hdcMem, hClipRgn);

	int firstVisible = _scrollPos / TRACK_HEIGHT;
	int lastVisible = min(firstVisible + ((_rcClient.bottom - TITLE_BAR_HEIGHT - CONTROL_PANEL_HEIGHT) / TRACK_HEIGHT) + 2,
		(int)_tracks.size());

	for (int i = firstVisible; i < lastVisible; ++i) {
		RECT trackRect = {
			0,
			TITLE_BAR_HEIGHT + i * TRACK_HEIGHT - _scrollPos,
			_rcClient.right - 10,
			TITLE_BAR_HEIGHT + (i + 1) * TRACK_HEIGHT - _scrollPos
		};

		if (trackRect.bottom > TITLE_BAR_HEIGHT && trackRect.top < _controlPanelRect.top) {
			DrawTrack(hdcMem, trackRect, _tracks[i], _trackStates[i], i == _selectedTrackIndex);
		}
	}

	SelectClipRgn(hdcMem, NULL);
	DeleteObject(hClipRgn);

	// 绘制控制面板
	DrawControlPanel(hdcMem);

	// 绘制滚动条
	DrawScrollBar(hdcMem);

	// 最终绘制到屏幕
	BitBlt(hdc, 0, 0, _rcClient.right, _rcClient.bottom, hdcMem, 0, 0, SRCCOPY);

	// 清理资源
	SelectObject(hdcMem, hbmOld);
	DeleteObject(hbmMem);
	DeleteDC(hdcMem);
	EndPaint(hwnd, &ps);
}

void MainUI::onWMMouseMove(HWND hwnd)
{

	if (!_isMouseTracking) {
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_HOVER, hwnd, HOVER_DEFAULT };
		TrackMouseEvent(&tme);
		_isMouseTracking = true;
	}

	//鼠标在标题栏上
	onMouseMove_TittleBar(hwnd);
	//鼠标在播放控制面板上
	onMouseMove_ControlPanel(hwnd);
	//鼠标在列表上
	onMouseMove_ListAndScroll(hwnd);
}

void MainUI::onMouseMove_ControlPanel(HWND hwnd)
{
	bool needRepaint = false;

	if (_lastMousePos.y > _controlPanelRect.top) {
		int btnSize = 32;
		int btnSpacing = 20;
		int totalWidth = btnSize * 5 + btnSpacing * 4;
		int startX = (_controlPanelRect.right - totalWidth) / 2;
		int centerY = (_controlPanelRect.top + _controlPanelRect.bottom) / 2;
		bool needRepaint = false;

		//展示模式按钮
		RECT displayModeBtnRect = {
			startX,
			centerY - btnSize / 2,
			startX + btnSize,
			centerY + btnSize / 2
		};
		bool displayModeHovered = PtInRect(&displayModeBtnRect, _lastMousePos);
		if (displayModeHovered != _modeBtnState.hovered) {
			_modeBtnState.hovered = displayModeHovered;
			needRepaint = true;
		}

		// Previous button
		RECT prevBtnRect = {
			displayModeBtnRect.right + btnSpacing,
			centerY - btnSize / 2,
			displayModeBtnRect.right + btnSpacing + btnSize,
			centerY + btnSize / 2
		};
		bool prevHovered = PtInRect(&prevBtnRect, _lastMousePos);
		if (prevHovered != _prevBtnState.hovered) {
			_prevBtnState.hovered = prevHovered;
			needRepaint = true;
		}

		// Play/Pause button
		RECT playBtnRect = {
			prevBtnRect.right + btnSpacing,
			centerY - btnSize / 2,
			prevBtnRect.right + btnSpacing + btnSize,
			centerY + btnSize / 2
		};
		bool playHovered = PtInRect(&playBtnRect, _lastMousePos);
		if (playHovered != _playBtnState.hovered) {
			_playBtnState.hovered = playHovered;
			needRepaint = true;
		}

		// Next button
		RECT nextBtnRect = {
			playBtnRect.right + btnSpacing,
			centerY - btnSize / 2,
			playBtnRect.right + btnSpacing + btnSize,
			centerY + btnSize / 2
		};
		bool nextHovered = PtInRect(&nextBtnRect, _lastMousePos);
		if (nextHovered != _nextBtnState.hovered) {
			_nextBtnState.hovered = nextHovered;
			needRepaint = true;
		}

		// Add button
		RECT addBtnRect = {
			nextBtnRect.right + btnSpacing,
			centerY - btnSize / 2,
			nextBtnRect.right + btnSpacing + btnSize,
			centerY + btnSize / 2
		};
		bool addHovered = PtInRect(&addBtnRect, _lastMousePos);
		if (addHovered != _addBtnState.hovered) {
			_addBtnState.hovered = addHovered;
			needRepaint = true;
		}

		if (needRepaint) {
			InvalidateRect(hwnd, &_controlPanelRect, FALSE);
		}
	}
}

void MainUI::mouseMove_List(HWND hwnd)
{
	bool newThumbHovered = PtInRect(&_scrollThumbRect, _lastMousePos);
	if (newThumbHovered != _scrollThumbHovered) {
		_scrollThumbHovered = newThumbHovered;
		InvalidateRect(hwnd, &_scrollTrackRect, FALSE);
	}

	int hoverIndex = (_lastMousePos.y - TITLE_BAR_HEIGHT + _scrollPos) / TRACK_HEIGHT;
	bool needTrackRepaint = false;

	for (size_t i = 0; i < _trackStates.size(); ++i) {
		bool shouldHover = (i == hoverIndex) && hoverIndex >= 0 &&
			hoverIndex < (int)_tracks.size() &&
			!PtInRect(&_scrollTrackRect, _lastMousePos) &&
			(_lastMousePos.y > TITLE_BAR_HEIGHT) &&
			(_lastMousePos.y < _controlPanelRect.top);

		if (_trackStates[i].isHovered != shouldHover && i != _selectedTrackIndex) {
			_trackStates[i].isHovered = shouldHover;
			needTrackRepaint = true;
		}
	}

	if (needTrackRepaint) {
		RECT rcClient;
		GetClientRect(hwnd, &rcClient);
		RECT rcInvalidate = rcClient;
		rcInvalidate.top = TITLE_BAR_HEIGHT;
		rcInvalidate.bottom = _controlPanelRect.top;
		InvalidateRect(hwnd, &rcInvalidate, FALSE);
	}
}

void MainUI::onWMMouseLeave(HWND hwnd)
{
	_isMouseTracking = false;
	bool needRepaint = false;

	for (auto& state : _trackStates) {
		if (state.isHovered) {
			state.isHovered = false;
			needRepaint = true;
		}
	}

	if (_scrollThumbHovered) {
		_scrollThumbHovered = false;
		needRepaint = true;
	}

	if (_titleBarHovered) {
		_titleBarHovered = false;
		needRepaint = true;
	}

	if (_closeBtnHovered) {
		_closeBtnHovered = false;
		needRepaint = true;
	}

	if (_prevBtnHovered || _playBtnHovered || _nextBtnHovered || _addBtnHovered) {
		_prevBtnHovered = false;
		_playBtnHovered = false;
		_nextBtnHovered = false;
		_addBtnHovered = false;
		needRepaint = true;
	}
	if (_prevBtnState.hovered || _playBtnState.hovered ||
		_nextBtnState.hovered || _addBtnState.hovered) {
		_prevBtnState.hovered = false;
		_playBtnState.hovered = false;
		_nextBtnState.hovered = false;
		_addBtnState.hovered = false;
		needRepaint = true;
	}

	if (needRepaint) {
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

void MainUI::onWMLButtonDown(HWND hwnd)
{
	RECT closeBtnRect = {
				_rcClient.right - CLOSE_BTN_SIZE - 10,
				(TITLE_BAR_HEIGHT - CLOSE_BTN_SIZE) / 2,
				_rcClient.right - 10,
				(TITLE_BAR_HEIGHT + CLOSE_BTN_SIZE) / 2
	};
	if (PtInRect(&closeBtnRect, _lastMousePos) && !_isDragging) {
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return ;
	}

	if (_lastMousePos.y <= TITLE_BAR_HEIGHT && !_scrollDragging) {
		_dragStartPoint = _lastMousePos;
		ClientToScreen(hwnd, &_dragStartPoint);
		POINT curPt;
		GetCursorPos(&curPt);
		_dragStartPoint.x = curPt.x - _dragStartPoint.x;
		_dragStartPoint.y = curPt.y - _dragStartPoint.y;
		_isDragging = true;
		SetCapture(hwnd);
		return ;
	}

	if (PtInRect(&_scrollThumbRect, _lastMousePos) && !_isDragging) {
		_scrollDragging = true;
		_scrollDragStartY = _lastMousePos.y;
		_scrollThumbStartY = _scrollThumbRect.top;
		SetCapture(hwnd);
	}
	else if (PtInRect(&_scrollTrackRect, _lastMousePos) && !_isDragging) {
		if (_lastMousePos.y < _scrollThumbRect.top) {
			ScrollTo(_scrollPos - ((_rcClient.bottom - TITLE_BAR_HEIGHT - CONTROL_PANEL_HEIGHT) / 2));
		}
		else {
			ScrollTo(_scrollPos + ((_rcClient.bottom - TITLE_BAR_HEIGHT - CONTROL_PANEL_HEIGHT) / 2));
		}
	}
	else if (_lastMousePos.y > _controlPanelRect.top) {
		leftButtonDown_ControlPanel(hwnd);
		return;
	}
	else if (!_isDragging && !_scrollDragging) {
		int clickedIndex = (_lastMousePos.y - TITLE_BAR_HEIGHT + _scrollPos) / TRACK_HEIGHT;

		if (clickedIndex >= 0 && clickedIndex < (int)_tracks.size()) {
			OnTrackClick(clickedIndex);
		}
	}
}

void MainUI::leftButtonDown_ControlPanel(HWND hwnd)
{
	int btnSize = 32;
	int btnSpacing = 20;
	int totalWidth = btnSize * 5 + btnSpacing * 4;
	int startX = (_controlPanelRect.right - totalWidth) / 2;
	int centerY = (_controlPanelRect.top + _controlPanelRect.bottom) / 2;

	//展示模式按钮
	RECT displayModeBtnRect = {
		startX,
		centerY - btnSize / 2,
		startX + btnSize,
		centerY + btnSize / 2
	};
	if (PtInRect(&displayModeBtnRect, _lastMousePos)) {
		_modeBtnState.pressed = true;
		InvalidateRect(hwnd, &_controlPanelRect, FALSE);
		SwitchDisplayMode();
		return;
	}
	// Previous button
	RECT prevBtnRect = {
		displayModeBtnRect.right + btnSpacing,
		centerY - btnSize / 2,
		displayModeBtnRect.right + btnSpacing + btnSize,
		centerY + btnSize / 2
	};
	if (PtInRect(&prevBtnRect, _lastMousePos)) {
		_prevBtnState.pressed = true;
		InvalidateRect(hwnd, &_controlPanelRect, FALSE);
		PlayPrevious();
		return;
	}

	// Play/Pause button
	RECT playBtnRect = {
		prevBtnRect.right + btnSpacing,
		centerY - btnSize / 2,
		prevBtnRect.right + btnSpacing + btnSize,
		centerY + btnSize / 2
	};
	if (PtInRect(&playBtnRect, _lastMousePos)) {
		_playBtnState.pressed = true;
		InvalidateRect(hwnd, &_controlPanelRect, FALSE);
		if (_isPlaying) {
			PausePlay();
		}
		else {
			ResumePlay();
		}
		return;
	}

	// Next button
	RECT nextBtnRect = {
		playBtnRect.right + btnSpacing,
		centerY - btnSize / 2,
		playBtnRect.right + btnSpacing + btnSize,
		centerY + btnSize / 2
	};
	if (PtInRect(&nextBtnRect, _lastMousePos)) {
		_nextBtnState.pressed = true;
		InvalidateRect(hwnd, &_controlPanelRect, FALSE);
		PlayNext();
		return ;
	}

	// Add button
	RECT addBtnRect = {
		nextBtnRect.right + btnSpacing,
		centerY - btnSize / 2,
		nextBtnRect.right + btnSpacing + btnSize,
		centerY + btnSize / 2
	};
	if (PtInRect(&addBtnRect, _lastMousePos)) {
		_addBtnState.pressed = true;
		InvalidateRect(hwnd, &_controlPanelRect, FALSE);
		AddMusicFiles();
		return;
	}
}
void MainUI::onMouseMove_ListAndScroll(HWND hwnd)
{
	if (_scrollDragging) {
		int deltaY = _lastMousePos.y - _scrollDragStartY;
		int newThumbY = _scrollThumbStartY + deltaY;
		newThumbY = max(0, min(_scrollTrackRect.bottom - _scrollThumbHeight, newThumbY));

		float ratio = (float)newThumbY / (_scrollTrackRect.bottom - _scrollThumbHeight);
		ScrollTo((int)(ratio * _scrollMax));
	}
	else if (_isDragging) {
		POINT curPt;
		GetCursorPos(&curPt);
		SetWindowPos(hwnd, NULL,
			curPt.x - _dragStartPoint.x,
			curPt.y - _dragStartPoint.y,
			0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	else {
		mouseMove_List(hwnd);
	}
}

void MainUI::onMouseMove_TittleBar(HWND hwnd)
{
	bool newTitleBarHovered = (_lastMousePos.y <= TITLE_BAR_HEIGHT);
	if (newTitleBarHovered != _titleBarHovered) {
		_titleBarHovered = newTitleBarHovered;
		RECT titleBarRect = { 0, 0, _rcClient.right, TITLE_BAR_HEIGHT };
		InvalidateRect(hwnd, &titleBarRect, FALSE);
	}

	RECT closeBtnRect = {
		_rcClient.right - CLOSE_BTN_SIZE - 10,
		(TITLE_BAR_HEIGHT - CLOSE_BTN_SIZE) / 2,
		_rcClient.right - 10,
		(TITLE_BAR_HEIGHT + CLOSE_BTN_SIZE) / 2
	};
	bool newCloseBtnHovered = PtInRect(&closeBtnRect, _lastMousePos);
	if (newCloseBtnHovered != _closeBtnHovered) {
		_closeBtnHovered = newCloseBtnHovered;
		InvalidateRect(hwnd, &closeBtnRect, FALSE);
	}
}
