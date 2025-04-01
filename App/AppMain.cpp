#pragma once
#include <windows.h>
#include <thread>

#include "MainUI.h"
#include "RenderingThread.h"
#include "SceneManager.h"
#include "RoundedWindow.h"

// **彻底去掉窗口边框 + 禁用所有阴影**
void makeWindowBorderless(HWND hWnd) {
	// 去掉窗口装饰和边框
	LONG style = GetWindowLong(hWnd, GWL_STYLE);
	style &= ~(WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_THICKFRAME | WS_BORDER | WS_DLGFRAME);
	SetWindowLong(hWnd, GWL_STYLE, style);

	// 去掉扩展样式中的阴影
	LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW; // 透明窗口 + 置顶 + 工具窗口
	SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);

	// 禁用 DWM 阴影
	MARGINS margins = { -1, -1, -1, -1 }; // 彻底关闭窗口阴影
	DwmExtendFrameIntoClientArea(hWnd, &margins);

	// 设置窗口完全透明
	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_COLORKEY);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// 提前初始化GDI+
	ULONG_PTR g_gdiplusToken = 0;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) {
		return -1;
	}

	setlocale(LC_ALL, "");
	InitCommonControls();

	SceneManager sceneManager;
	// **获取窗口句柄并去掉边框 & 阴影**
	HWND hwnd = sceneManager.getHWND();
	if (hwnd) {
		makeWindowBorderless(hwnd);  // 去掉边框 + 透明设置
	}

	RenderingThread* renderingThread = new RenderingThread(sceneManager.viewer);
	renderingThread->start();

	MainUI mainUI(NULL);
	mainUI.Show();

	// 创建圆角窗口
	RoundedWindow window;
	if (!window.Show(328, 492, 23)) return -1; // 宽度, 高度, 圆角半径

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 程序退出时清理
	GdiplusShutdown(g_gdiplusToken);
	CoUninitialize();
	return 0;
}