#pragma once
#include <windows.h>
#include <thread>

#include "MainUI.h"
#include "RenderingThread.h"
#include "SceneManager.h"
#include "RoundedWindow.h"

// **����ȥ�����ڱ߿� + ����������Ӱ**
void makeWindowBorderless(HWND hWnd) {
	// ȥ������װ�κͱ߿�
	LONG style = GetWindowLong(hWnd, GWL_STYLE);
	style &= ~(WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_THICKFRAME | WS_BORDER | WS_DLGFRAME);
	SetWindowLong(hWnd, GWL_STYLE, style);

	// ȥ����չ��ʽ�е���Ӱ
	LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW; // ͸������ + �ö� + ���ߴ���
	SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);

	// ���� DWM ��Ӱ
	MARGINS margins = { -1, -1, -1, -1 }; // ���׹رմ�����Ӱ
	DwmExtendFrameIntoClientArea(hWnd, &margins);

	// ���ô�����ȫ͸��
	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_COLORKEY);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// ��ǰ��ʼ��GDI+
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
	// **��ȡ���ھ����ȥ���߿� & ��Ӱ**
	HWND hwnd = sceneManager.getHWND();
	if (hwnd) {
		makeWindowBorderless(hwnd);  // ȥ���߿� + ͸������
	}

	RenderingThread* renderingThread = new RenderingThread(sceneManager.viewer);
	renderingThread->start();

	MainUI mainUI(NULL);
	mainUI.Show();

	// ����Բ�Ǵ���
	RoundedWindow window;
	if (!window.Show(328, 492, 23)) return -1; // ���, �߶�, Բ�ǰ뾶

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// �����˳�ʱ����
	GdiplusShutdown(g_gdiplusToken);
	CoUninitialize();
	return 0;
}