#include "SongListUI.h"

SongListUI::SongListUI(HWND hwndParent) :
	_hwndParent(hwndParent),
	_isDragging(false),
	_hoveredRow(-1) {

	// 初始化表格项目
	_items = {
		"开始游戏",
		"游戏设置",
		"读取存档",
		"多人模式",
		"退出游戏"
	};

	// 创建颜色和字体
	_normalBrush = CreateSolidBrush(RGB(50, 50, 50));
	_hoverBrush = CreateSolidBrush(RGB(80, 80, 80));

	_hFont = CreateFont(
		20, 0, 0, 0, FW_NORMAL,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		"Microsoft YaHei"
	);

	// 注册自定义窗口类
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ListProc;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = "InteractiveTableClass";
	RegisterClassEx(&wcex);

	// 创建表格窗口
	_hwndTable = CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
		"InteractiveTableClass",
		"Menu",
		WS_POPUP,
		300, 200, 250, 250,
		hwndParent, NULL,
		GetModuleHandle(NULL),
		this
	);

	// 设置透明度
	SetLayeredWindowAttributes(_hwndTable, 0, 220, LWA_ALPHA);
}

SongListUI::~SongListUI()
{
	DeleteObject(_normalBrush);
	DeleteObject(_hoverBrush);
	DeleteObject(_hFont);
}

HWND SongListUI::getHWND()
{
	return _hwndTable;
}

void SongListUI::show()
{
	ShowWindow(_hwndTable, SW_SHOW);
	UpdateWindow(_hwndTable);
}

void SongListUI::Hide()
{
	ShowWindow(_hwndTable, SW_HIDE);
}

LRESULT SongListUI::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_PAINT:
		return OnPaint(hwnd);

	case WM_LBUTTONDOWN:
		return OnLButtonDown(hwnd, lParam);

	case WM_LBUTTONUP:
		return OnLButtonUp(hwnd);

	case WM_MOUSEMOVE:
		return OnMouseMove(hwnd, wParam, lParam);

	case WM_MOUSELEAVE:
		_hoveredRow = -1;
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(hwnd, lParam);

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

LRESULT SongListUI::OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	// 使用双缓冲绘制
	HDC hdcBuffer = CreateCompatibleDC(hdc);
	RECT rc;
	GetClientRect(hwnd, &rc);
	HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);

	// 填充背景
	FillRect(hdcBuffer, &rc, _normalBrush);

	// 选择字体
	HFONT hOldFont = (HFONT)SelectObject(hdcBuffer, _hFont);
	SetBkMode(hdcBuffer, TRANSPARENT);

	// 绘制每一行
	for (size_t i = 0; i < _items.size(); ++i) {
		RECT rowRect = { 0, (int)(i * 50), rc.right, (int)((i + 1) * 50) };

		// 高亮悬停行
		if (i == _hoveredRow) {
			FillRect(hdcBuffer, &rowRect, _hoverBrush);
		}

		// 绘制文字
		SetTextColor(hdcBuffer, RGB(255, 255, 255));
		DrawTextA(hdcBuffer, _items[i].c_str(), -1, &rowRect,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	// 恢复原始对象
	SelectObject(hdcBuffer, hOldFont);

	// 将缓冲区内容复制到屏幕
	BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcBuffer, 0, 0, SRCCOPY);

	// 清理
	SelectObject(hdcBuffer, hbmOldBuffer);
	DeleteObject(hbmBuffer);
	DeleteDC(hdcBuffer);

	EndPaint(hwnd, &ps);
	return 0;
}

LRESULT SongListUI::OnLButtonDown(HWND hwnd, LPARAM lParam)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	// 开始拖动
	_isDragging = true;
	_dragOffset = pt;
	SetCapture(hwnd);
	return 0;
}

LRESULT SongListUI::OnLButtonUp(HWND hwnd)
{
	ReleaseCapture();
	_isDragging = false;
	return 0;
}

LRESULT SongListUI::OnMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	RECT rc;
	GetClientRect(hwnd, &rc);

	// 处理拖动
	if (_isDragging && (wParam & MK_LBUTTON)) {
		POINT curPt;
		GetCursorPos(&curPt);
		SetWindowPos(hwnd, NULL,
			curPt.x - _dragOffset.x,
			curPt.y - _dragOffset.y,
			0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return 0;
	}

	// 处理行悬停效果
	int newHoveredRow = pt.y / 50;
	if (newHoveredRow >= 0 && newHoveredRow < (int)_items.size()) {
		if (newHoveredRow != _hoveredRow) {
			_hoveredRow = newHoveredRow;
			InvalidateRect(hwnd, NULL, TRUE);
		}
	}

	// 追踪鼠标离开事件
	TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = hwnd;
	TrackMouseEvent(&tme);

	return 0;
}

LRESULT SongListUI::OnLButtonDblClk(HWND hwnd, LPARAM lParam)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	int clickedRow = pt.y / 50;

	if (clickedRow >= 0 && clickedRow < (int)_items.size()) {
		switch (clickedRow) {
		case 0: // 开始游戏
			MessageBox(hwnd, "游戏开始!", "提示", MB_OK);
			break;
		case 1: // 游戏设置
			MessageBox(hwnd, "打开设置", "提示", MB_OK);
			break;
		case 2: // 读取存档
			MessageBox(hwnd, "读取存档", "提示", MB_OK);
			break;
		case 3: // 多人模式
			MessageBox(hwnd, "多人模式", "提示", MB_OK);
			break;
		case 4: // 退出游戏
			PostQuitMessage(0);
			break;
		}
	}
	return 0;
}

LRESULT CALLBACK SongListUI::ListProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SongListUI* pThis = nullptr;

	if (msg == WM_NCCREATE) {
		CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
		pThis = reinterpret_cast<SongListUI*>(pCS->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	else {
		pThis = reinterpret_cast<SongListUI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (pThis) {
		return pThis->HandleMessage(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
