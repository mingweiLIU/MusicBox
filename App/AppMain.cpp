#pragma once
#include <windows.h>
#include "MiniAppUI.h"
#include "RenderingThread.h"
#include "SceneManager.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    SceneManager sceneManager;
    RenderingThread* renderingThread = new RenderingThread(sceneManager.viewer);
    renderingThread->start();

    // 创建圆角窗口
    MiniAppUI appMiniUI(sceneManager.getHWND(),sceneManager);
    if (!appMiniUI.Show()) {
        MessageBox(nullptr, "圆角窗口创建失败", "错误", MB_ICONERROR);
        return 1;
    }

	sceneManager.getCamera();
	sceneManager.setCamera();
	sceneManager.getCamera();
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    // 程序退出时停止渲染线程
    renderingThread->cancel();
    delete renderingThread;  // 释放内存

    return (int)msg.wParam;
}
 