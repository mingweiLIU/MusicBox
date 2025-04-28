#include "MiniAppUI.h"

#include <windowsx.h>
#include <filesystem>
#include <algorithm>
#include "SoundBox.h"

MiniAppUI::MiniAppUI(HWND parent, SceneManager& sceneManager,int width, int height, int cornerRadius )
    : m_hwnd(nullptr), m_parent(parent), m_cornerRadius(cornerRadius), sceneManager(sceneManager),
    m_bgColor(D2D1::ColorF(0.2f, 0.2f, 0.2f, 0.6f)),
    m_borderColor(D2D1::ColorF(0.27f, 0.51f, 0.78f, 1.0f)),
    m_borderWidth(1.5f),
    m_pD2DFactory(nullptr),
    m_pRenderTarget(nullptr),
    m_pBgBrush(nullptr),
    m_pBorderBrush(nullptr) {
    // Initialize Direct2D factory
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

    // Register window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW| CS_DBLCLKS;
    wc.lpfnWndProc = MiniAppUI::WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "RoundedWindowD2DClass";
    if (!RegisterClassEx(&wc)) return ;

    //计算位置
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create layered window
    int windowInitX = screenWidth - 2 * width; int windowInitY = 4 * height;
    m_hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        "RoundedWindowD2DClass",
        "Rounded Window D2D",
        WS_POPUP,
        windowInitX , windowInitY,//这里设置后 其实会在后续的界面更新函数中变更 所以其实应该通过actualWindowPos来控制
        width, height,
        m_parent,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );
    if (!m_hwnd) return ;

    //初始化一些位置
    actualWindowPos = POINT{ (LONG)windowInitX, (LONG)windowInitY };//初始化位置设置
    lastMousePos = POINT();
	//启动监听
	player = new Player(m_hwnd,false, tracks);
    //设置一些更新函数
	spectumTimer = new SpectumTimer(player);
	spectumTimer->Start();

    HRESULT hr = CreateDeviceResources();
    if (FAILED(hr)) return;
}

MiniAppUI::~MiniAppUI() {
    Close();
    DiscardDeviceResources();
	SafeRelease(&m_pD2DFactory);
	if (spectumTimer) {
		spectumTimer->Stop();
		delete spectumTimer;
	}
	if (player) delete player;
}

bool MiniAppUI::Show() {
    // Initial render
    UpdateWindowAppearance();
    ShowWindow(m_hwnd, SW_SHOW);
    return true;
}

void MiniAppUI::Close() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void MiniAppUI::SetBackgroundColor(D2D1_COLOR_F color) {
    m_bgColor = color;
    if (m_pBgBrush) {
        m_pBgBrush->SetColor(color);
    }
    if (m_hwnd) UpdateWindowAppearance();
}

void MiniAppUI::SetBorderColor(D2D1_COLOR_F color, float width) {
    m_borderColor = color;
    m_borderWidth = width;
    if (m_pBorderBrush) {
        m_pBorderBrush->SetColor(color);
    }
    if (m_hwnd) UpdateWindowAppearance();
}

void MiniAppUI::Resize(int width, int height) {
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
        if (m_pRenderTarget) {
            m_pRenderTarget->Resize(D2D1::SizeU(width, height));
        }
        UpdateWindowAppearance();
    }
}

LRESULT CALLBACK MiniAppUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        MiniAppUI* pThis = reinterpret_cast<MiniAppUI*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    MiniAppUI* pThis = reinterpret_cast<MiniAppUI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return pThis ? pThis->HandleMessage(hwnd, msg, wParam, lParam) : DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT MiniAppUI::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_LBUTTONDOWN: {
        return 0;
    }
	case WM_LBUTTONUP: {
		lastMousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		onWMLButtonDown(hwnd);
        return 0;
    }
    case WM_PAINT:
        UpdateWindowAppearance();
        break;
    case WM_MOVING: {
        RECT* pRect = reinterpret_cast<RECT*>(lParam);
        actualWindowPos.x = pRect->left;
        actualWindowPos.y = pRect->top;
        return TRUE;
    }
    case WM_MOUSEMOVE: {
		lastMousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        onWMMouseMove(hwnd);
        return 0;
    }
    case WM_MOUSELEAVE: {
        onWMMouseLeave(hwnd);
        return 0;
    }
    case WM_SIZE:
        if (m_pRenderTarget) {
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            m_pRenderTarget->Resize(D2D1::SizeU(rc.right, rc.bottom));
            UpdateWindowAppearance();
        }
        break;

    case WM_NCHITTEST: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);

        bool inControlBts = false;
        for (auto& oneButton:controlBtns)
        {
            if (oneButton.isInBtn(pt)) {
                inControlBts = true;
                break;
            }
        }
		if (inControlBts) {
			return HTCLIENT;
        }
        else {
            return HTCAPTION;
        }
        return HTCLIENT;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HRESULT MiniAppUI::CreateDeviceResources() {
    HRESULT hr = S_OK;

    if (!m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        // Create a Direct2D render target
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
        );

        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(m_bgColor, &m_pBgBrush);
        }

        if (SUCCEEDED(hr)) {
            hr = m_pRenderTarget->CreateSolidColorBrush(m_borderColor, &m_pBorderBrush);
        }

        //创建内容对象
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            0, 0
        );

        hr = m_pD2DFactory->CreateDCRenderTarget(&props, &m_pDCRenderTarget);
        m_pDCRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

        hr = m_pDCRenderTarget->CreateSolidColorBrush(m_bgColor, &m_trackBgBrush);


        //titlebar
        // 创建DirectWrite工厂
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
        );

        //创建titlebar的东西
        if (SUCCEEDED(hr)) {
            // 创建文本格式
            hr = m_pDWriteFactory->CreateTextFormat(
                L"微软雅黑,SimSun, Microsoft YaHei, Arial",                // 字体家族名
                nullptr,                   // 字体集合(nullptr表示系统默认)
                DWRITE_FONT_WEIGHT_SEMI_BOLD, // 字体粗细
                DWRITE_FONT_STYLE_NORMAL,  // 字体样式
                DWRITE_FONT_STRETCH_NORMAL,// 字体拉伸
                18.0f,                     // 字体大小
                L"zh-cn",                  // 区域设置
                &m_titleBarTextFormat
            );
            if (SUCCEEDED(hr)) {
                m_titleBarTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                m_titleBarTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            }
            if (SUCCEEDED(hr)) {
                // 创建文字画刷
                hr = m_pDCRenderTarget->CreateSolidColorBrush(m_titleBarTextColor, &m_tileBarTextBrush);
            }
        }        
    }

    return hr;
}

void MiniAppUI::DiscardDeviceResources() {
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBgBrush);
    SafeRelease(&m_pBorderBrush);
    SafeRelease(&m_pDCRenderTarget);
    SafeRelease(&m_tileBarTextBrush);
    SafeRelease(&m_titleBarTextFormat);
    SafeRelease(&m_trackBgBrush);
}

void MiniAppUI::SwitchDisplayMode()
{
	// 转换为整数进行计算
	int next = static_cast<int>(m_displayMode) + 1;

	// 如果超过枚举值数量则循环回到第一个
	if (next >= static_cast<int>(SpectumMode::COUNT)) {
		next = 0;
	}

	// 转换回枚举类型
    m_displayMode = static_cast<SpectumMode>(next);

}

void MiniAppUI::PausePlay()
{
}

void MiniAppUI::ResumePlay()
{
	// Here you would resume the actual audio playback
	if (player->isPlaying()) {
		player->PauseResumSong();
	}
	else {
        //player->PlaySong(m_playingIndex, m_trackLength);
	}
}

void MiniAppUI::onWMMouseMove(HWND hwnd)
{
    if (!isMouseTracking) {
        TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_HOVER, hwnd, HOVER_DEFAULT };
        TrackMouseEvent(&tme);
        isMouseTracking = true;
    }

    bool shouldUpdate = false;
    for (auto& oneControlBtn : controlBtns)
    {
        if (oneControlBtn.isInBtn(lastMousePos)&& oneControlBtn.state != State::HOVERED)
        {
            oneControlBtn.state = State::HOVERED;
            shouldUpdate = true;
        }
        if(!oneControlBtn.isInBtn(lastMousePos) && oneControlBtn.state == State::HOVERED) {
            oneControlBtn.state = State::NONE;
            shouldUpdate = true;
        }
    }
    if(shouldUpdate)InvalidateRect(hwnd, &rcClient, FALSE);
}

void MiniAppUI::onWMMouseLeave(HWND hwnd)
{
    isMouseTracking = false;
    bool shouldUpdate = false;
    for (auto& oneControlBtn : controlBtns)
    {
        if (oneControlBtn.state != State::NONE)
        {
            oneControlBtn.state = State::NONE;
            shouldUpdate = true;
            break;
        }
    }
    if (shouldUpdate)InvalidateRect(hwnd, &rcClient, FALSE);
}

void MiniAppUI::onWMLButtonDown(HWND hwnd)
{
    bool shouldUpdate = false;
	for (auto& oneControlBtn : controlBtns)
	{
		if (!oneControlBtn.isInBtn(lastMousePos)) continue;
		//下面根据实际的按钮情况来处理
		switch (oneControlBtn.type)
		{
        case ButtonType::CubeMode:case ButtonType::CubeDropMode: case ButtonType::WaveMode:  case ButtonType::WaveDropMode: case ButtonType::DoubleWaveMode: {
			
			// 转换为整数进行计算
			int next = static_cast<int>(m_displayMode) + 1;

			// 如果超过枚举值数量则循环回到第一个
			if (next == static_cast<int>(SpectumMode::COUNT)) {
				next = 0;
			}

			// 转换回枚举类型
			m_displayMode = static_cast<SpectumMode>(next);
            shouldUpdate = true;

			sceneManager.setDisplayMode(m_displayMode);
            
			break;
		}
		case ButtonType::Play: case ButtonType::Pause : case ButtonType::Stop : {
            
            if (oneControlBtn.type == ButtonType::Play) {
                if (!player->isPlaying()) {
                    player->initBASS();
                    player->RecordSystemSound();
                }
                else {
                    player->PauseResumSong();
                }
            }
            else if (oneControlBtn.type == ButtonType::Pause) {
                player->PauseSong();
            }
            else if (oneControlBtn.type == ButtonType::Stop) {
                player->StopSongAndFree();
                for (size_t i = 0; i < 100; i++)
                {
                    *(ChangeSoundBoxHeight::spectum + i) = 20;
                }
            }

            // 转换为整数进行计算
            int next = static_cast<int>(m_controlMode) + 1;

            // 如果超过枚举值数量则循环回到第一个
            if (next == static_cast<int>(ControlMode::COUNT)) {
                next = 0;
            }

            // 转换回枚举类型
            m_controlMode = static_cast<ControlMode>(next);
            shouldUpdate = true;
			break;
		}		
		case ButtonType::Close: {
			//退出
			shouldUpdate = true;
			break;
		}
		}
	} 
    if (shouldUpdate)
    {
        InvalidateRect(hwnd, &rcClient, false);
    }
}


void MiniAppUI::UpdateWindowAppearance() {
    if (!m_hwnd) return;

    // Create memory DC and DIB section
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    GetClientRect(m_hwnd, &rcClient);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = rcClient.right;
    bmi.bmiHeader.biHeight = -rcClient.bottom;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits;
    HBITMAP hbmMem = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    SelectObject(hdcMem, hbmMem);

    // Create and bind DC render target
    HRESULT hr = m_pDCRenderTarget->BindDC(hdcMem, &rcClient);
    //if (FAILED(hr)) return;
    m_pDCRenderTarget->BeginDraw();
    m_pDCRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pDCRenderTarget->Clear(D2D1::ColorF(0, 0.0f));

    // 绘制所有元素
    DrawAll(m_pDCRenderTarget);

    hr = m_pDCRenderTarget->EndDraw();

    // Update layered window
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    POINT ptSrc = { 0, 0 };
    SIZE size = { rcClient.right, rcClient.bottom };
    POINT ptDst;
    ptDst = { actualWindowPos.x, actualWindowPos.y };
   
    UpdateLayeredWindow(m_hwnd, nullptr, &ptDst, &size, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    // Clean up
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

void MiniAppUI::DrawAll(ID2D1RenderTarget* pRenderTarget) {
    DrawMiniFrame(pRenderTarget);
    //DrawRoundedFrame(pRenderTarget);
    DrawControlBtns(pRenderTarget);
}

void MiniAppUI::DrawMiniFrame(ID2D1RenderTarget* pRenderTarget) {
     //创建路径几何对象
    ID2D1PathGeometry* pPathGeometry = nullptr;
    ID2D1Factory* pFactory = nullptr;
    pRenderTarget->GetFactory(&pFactory);

    HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
    if (SUCCEEDED(hr))
    {
        ID2D1GeometrySink* pSink = nullptr;
        hr = pPathGeometry->Open(&pSink);

        if (SUCCEEDED(hr))
        {
            float offset = 5;
            float bigCircleRadius = offset + m_BtnCircleRadius;
            float crossDis = sqrt(bigCircleRadius * bigCircleRadius - m_BtnRadius * m_BtnRadius);
            float crossX = bigCircleRadius + crossDis;
            // 开始绘制路径
            pSink->BeginFigure(D2D1::Point2F(0,bigCircleRadius), D2D1_FIGURE_BEGIN_FILLED);
            pSink->AddArc(D2D1::ArcSegment(
                D2D1::Point2F(bigCircleRadius, 2*bigCircleRadius),
                D2D1::SizeF(bigCircleRadius, bigCircleRadius),
                0.0f,
                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                D2D1_ARC_SIZE_SMALL
            ));
            // 顶部左边圆角
            pSink->AddArc(D2D1::ArcSegment(
                D2D1::Point2F(crossX, bigCircleRadius+m_BtnRadius),
                D2D1::SizeF(bigCircleRadius, bigCircleRadius),
                0.0f,
                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                D2D1_ARC_SIZE_SMALL
            ));

            pSink->AddLine(D2D1::Point2F(rcClient.right - m_BtnRadius, bigCircleRadius + m_BtnRadius));

            pSink->AddArc(D2D1::ArcSegment(
                D2D1::Point2F(rcClient.right, bigCircleRadius),
                D2D1::SizeF(m_BtnRadius, m_BtnRadius),
                0.0f,
                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                D2D1_ARC_SIZE_SMALL
            ));
            pSink->AddArc(D2D1::ArcSegment(
                D2D1::Point2F(rcClient.right - m_BtnRadius,bigCircleRadius-m_BtnRadius),
                D2D1::SizeF(m_BtnRadius, m_BtnRadius),
                0.0f,
                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                D2D1_ARC_SIZE_SMALL
            ));
            pSink->AddLine(D2D1::Point2F(crossX, bigCircleRadius - m_BtnRadius));
            pSink->AddArc(D2D1::ArcSegment(
                D2D1::Point2F(bigCircleRadius, 0),
                D2D1::SizeF(bigCircleRadius, bigCircleRadius),
                0.0f,
                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                D2D1_ARC_SIZE_SMALL
            ));           

            pSink->AddArc(D2D1::ArcSegment(
                D2D1::Point2F(0, bigCircleRadius),
                D2D1::SizeF(bigCircleRadius, bigCircleRadius),
                0.0f,
                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
                D2D1_ARC_SIZE_SMALL
            ));

            // 完成图形
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            pSink->Close();
            pSink->Release();

            // 填充路径
            ID2D1SolidColorBrush* pBgBrush = nullptr;
            HRESULT hr;

            if (FAILED(hr = pRenderTarget->CreateSolidColorBrush(m_bgColor, &pBgBrush))) {
                // Handle error (log, return, etc.)
                return;
            }
            pRenderTarget->FillGeometry(pPathGeometry, pBgBrush);
        }

        pPathGeometry->Release();
    }

    SafeRelease(&pFactory);
}

void MiniAppUI::DrawControlButton(ID2D1RenderTarget* pRenderTarget, ControlButton& controlBtn)
{
    //这个按钮要实现鼠标放上去放大 并且叠加一层泛光效果
    float scale = (controlBtn.state == State::HOVERED) ? 1.1f : 1.0f;
    int adjustedSize = (int)(controlBtn.size * scale);

    D2D1_COLOR_F btnColor=colorControlBtn;
    if (controlBtn.state == State::HOVERED) {
        btnColor = colorControlBtnHover;
    }else if (controlBtn.state==State::PRESSED)
    {
        btnColor = colorControlBtnPressed;
    }

    //绘制图标 因为图标要比按钮范围小 所以再缩放下
    float iconScale = 0.5f;
    float iconSize = adjustedSize * iconScale;
    float iconX = controlBtn.centerX - iconSize / 2;
    float iconY = controlBtn.centerY - iconSize / 2;
    m_trackBgBrush->SetColor(colorControlBtnIcon);

    switch (controlBtn.type)
    {
    case ButtonType::Play : {
        std::vector<D2D1_POINT_2F> geomPoints = {
            D2D1::Point2F(iconX,iconY),
            D2D1::Point2F(iconX+iconSize,iconY + iconSize / 2),
            D2D1::Point2F(iconX ,iconY + iconSize),
        };
        m_trackBgBrush->SetColor(colorControlBtnIcon);
        DrawGeometry(pRenderTarget, m_trackBgBrush, geomPoints);
        break;
    }
    case ButtonType::Pause: {
        m_trackBgBrush->SetColor(colorControlBtnIcon);
        float barWidth = iconSize / 3; float spacing = iconSize / 3;
        D2D1_RECT_F rect1 = D2D1::RectF(iconX+1, iconY, iconX + barWidth, iconY + iconSize);
        D2D1_RECT_F rect2 = D2D1::RectF(iconX+1+barWidth+spacing, iconY, iconX + 2*barWidth+spacing, iconY + iconSize);
        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
        pRenderTarget->FillRectangle(&rect2, m_trackBgBrush);

        //因为现在暂停按钮有个外部圆圈 所以其RECT需要更新
        int newRcRadius = 1.5 * iconSize;
        controlBtn.rc = RECT{controlBtn.centerX-newRcRadius,controlBtn.centerY-newRcRadius,controlBtn.centerX+newRcRadius,controlBtn.centerY+newRcRadius};

        break;
    }
    case ButtonType::Stop: {
        m_trackBgBrush->SetColor(colorControlBtnIcon);
        D2D1_RECT_F rect1 = D2D1::RectF(iconX, iconY, iconX + iconSize , iconY + iconSize);
        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
        break;
    }
    case ButtonType::Close: {
		int crossSize = 6;

		m_trackBgBrush->SetColor(colorControlBtnIcon);
		pRenderTarget->DrawLine(
			D2D1::Point2F(controlBtn.centerX - crossSize, controlBtn.centerY - crossSize),
			D2D1::Point2F(controlBtn.centerX + crossSize, controlBtn.centerY + crossSize),
			m_trackBgBrush, 2.0f
		);
		pRenderTarget->DrawLine(
			D2D1::Point2F(controlBtn.centerX + crossSize, controlBtn.centerY - crossSize),
			D2D1::Point2F(controlBtn.centerX - crossSize, controlBtn.centerY + crossSize),
			m_trackBgBrush, 2.0f
		);
        break;
    }
    case ButtonType::CubeMode: {
        m_trackBgBrush->SetColor(colorContolBtnPlayCicle);
        D2D1_ELLIPSE fillCicle = D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), m_BtnCircleRadius, m_BtnCircleRadius);
        pRenderTarget->FillEllipse(&fillCicle, m_trackBgBrush);

        float barWidth = iconSize / 8;
        float spacing = iconSize / 6;

        D2D1_RECT_F rect1 = D2D1::RectF(iconX, iconY + 2, iconX+barWidth, iconY+iconSize );
        D2D1_RECT_F rect2 = D2D1::RectF(iconX + barWidth + spacing, iconY, iconX + barWidth + spacing+barWidth, iconY + iconSize);
        D2D1_RECT_F rect3 = D2D1::RectF(iconX + (barWidth + spacing) * 2, iconY + 4, iconX + (barWidth + spacing) * 2+barWidth, iconY + iconSize);
        D2D1_RECT_F rect4 = D2D1::RectF(iconX + (barWidth + spacing) * 3, iconY + 2, iconX + (barWidth + spacing) * 3+barWidth, iconY + iconSize);

        m_trackBgBrush->SetColor(colorControlBtnIcon);
        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
        pRenderTarget->FillRectangle(&rect2, m_trackBgBrush);
        pRenderTarget->FillRectangle(&rect3, m_trackBgBrush);
        pRenderTarget->FillRectangle(&rect4, m_trackBgBrush);

		//因为现在播放按钮有个外部圆圈 所以其RECT需要更新
		int newRcRadius = 1.5 * iconSize;
		controlBtn.rc = RECT{ controlBtn.centerX - newRcRadius,controlBtn.centerY - newRcRadius,controlBtn.centerX + newRcRadius,controlBtn.centerY + newRcRadius };
        break;
    }
    case ButtonType::CubeDropMode: {
        m_trackBgBrush->SetColor(colorContolBtnPlayCicle);
        D2D1_ELLIPSE fillCicle = D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), m_BtnCircleRadius, m_BtnCircleRadius);
        pRenderTarget->FillEllipse(&fillCicle, m_trackBgBrush);

        float barWidth = iconSize / 8;
        float spacing = iconSize / 6;

        D2D1_RECT_F rect1 = D2D1::RectF(iconX, iconY + 6, iconX+barWidth, iconY + iconSize);
        D2D1_RECT_F rect2 = D2D1::RectF(iconX + barWidth + spacing, iconY + 8, iconX + barWidth + spacing+barWidth, iconY + iconSize);
        D2D1_RECT_F rect3 = D2D1::RectF(iconX + (barWidth + spacing) * 2, iconY + 4, iconX + (barWidth + spacing) * 2+barWidth, iconY + iconSize);
        D2D1_RECT_F rect4 = D2D1::RectF(iconX + (barWidth + spacing) * 3, iconY + 9, iconX + (barWidth + spacing) * 3+barWidth, iconY + iconSize);

        m_trackBgBrush->SetColor(colorControlBtnIcon);
        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
        pRenderTarget->FillRectangle(&rect2, m_trackBgBrush);
        pRenderTarget->FillRectangle(&rect3, m_trackBgBrush);
        pRenderTarget->FillRectangle(&rect4, m_trackBgBrush);

        std::vector<D2D1_POINT_2F> geomPoints = {
            D2D1::Point2F(iconX, iconY + 2),
            D2D1::Point2F(iconX + barWidth + spacing, iconY + 4),
            D2D1::Point2F(iconX + (barWidth + spacing) * 2, iconY),
            D2D1::Point2F(iconX + (barWidth + spacing) * 3 + 2, iconY + 5),
        };
        for (int i=0;i<3;++i)
        {
            pRenderTarget->DrawLine(geomPoints[i], geomPoints[i + 1], m_trackBgBrush, 2.0f);
        }

		//因为现在播放按钮有个外部圆圈 所以其RECT需要更新
		int newRcRadius = 1.5 * iconSize;
		controlBtn.rc = RECT{ controlBtn.centerX - newRcRadius,controlBtn.centerY - newRcRadius,controlBtn.centerX + newRcRadius,controlBtn.centerY + newRcRadius };
        break;
    }
    case ButtonType::WaveMode: {
		m_trackBgBrush->SetColor(colorContolBtnPlayCicle);
		D2D1_ELLIPSE fillCicle = D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), m_BtnCircleRadius, m_BtnCircleRadius);
		pRenderTarget->FillEllipse(&fillCicle, m_trackBgBrush);

		//绘制一层弧线
		m_trackBgBrush->SetColor(colorControlBtnIcon);
		DrawSin(pRenderTarget, m_trackBgBrush, D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), iconSize, iconSize, iconSize / 2, 4.0f);

        //因为现在播放按钮有个外部圆圈 所以其RECT需要更新
        int newRcRadius = 1.5 * iconSize;
        controlBtn.rc = RECT{ controlBtn.centerX - newRcRadius,controlBtn.centerY - newRcRadius,controlBtn.centerX + newRcRadius,controlBtn.centerY + newRcRadius };
        break;
    }

	case ButtonType::WaveDropMode: {
		m_trackBgBrush->SetColor(colorContolBtnPlayCicle);
		D2D1_ELLIPSE fillCicle = D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), m_BtnCircleRadius, m_BtnCircleRadius);
		pRenderTarget->FillEllipse(&fillCicle, m_trackBgBrush);

		//绘制一层弧线
		m_trackBgBrush->SetColor(colorControlBtnIcon);
		DrawCos(pRenderTarget, m_trackBgBrush, D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), iconSize, iconSize, -iconSize / 2.0, 4.0f);

		//因为现在播放按钮有个外部圆圈 所以其RECT需要更新
		int newRcRadius = 1.5 * iconSize;
		controlBtn.rc = RECT{ controlBtn.centerX - newRcRadius,controlBtn.centerY - newRcRadius,controlBtn.centerX + newRcRadius,controlBtn.centerY + newRcRadius };
		break;
	}
	case ButtonType::DoubleWaveMode: {

		m_trackBgBrush->SetColor(colorContolBtnPlayCicle);
		D2D1_ELLIPSE fillCicle = D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), m_BtnCircleRadius, m_BtnCircleRadius);
		pRenderTarget->FillEllipse(&fillCicle, m_trackBgBrush);

		//绘制一层弧线
		m_trackBgBrush->SetColor(colorControlBtnIcon);
		DrawSin(pRenderTarget, m_trackBgBrush, D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), iconSize, iconSize, iconSize / 2, 2.0f);
		DrawSin(pRenderTarget, m_trackBgBrush, D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), iconSize, iconSize, -iconSize / 2, 2.0f);

		//因为现在播放按钮有个外部圆圈 所以其RECT需要更新
		int newRcRadius = 1.5 * iconSize;
		controlBtn.rc = RECT{ controlBtn.centerX - newRcRadius,controlBtn.centerY - newRcRadius,controlBtn.centerX + newRcRadius,controlBtn.centerY + newRcRadius }; 
		break;
	}
    }
    // 当按钮被hover时显示泛光效果
    if (controlBtn.state==State::HOVERED) {
        // 创建径向渐变笔刷以实现发光效果
        ID2D1RadialGradientBrush* pRadialGradientBrush = nullptr;

        // 定义渐变停止点
        D2D1_GRADIENT_STOP gradientStops[2];
        gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::White, 0.2f); // 中心半透明白色
        gradientStops[0].position = 0.0f;
        gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::White, 0.0f); // 边缘完全透明
        gradientStops[1].position = 1.0f;

        ID2D1GradientStopCollection* pGradientStopCollection = nullptr;
        pRenderTarget->CreateGradientStopCollection(
            gradientStops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &pGradientStopCollection
        );

        // 计算扩展尺寸
        float expandedSize = adjustedSize * 1.2f;
        float radius = expandedSize / 2.0f;

        // 创建径向渐变笔刷
        pRenderTarget->CreateRadialGradientBrush(
            D2D1::RadialGradientBrushProperties(
                D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), // 中心点
                D2D1::Point2F(0, 0), // 偏移量
                radius, radius),     // 半径
            pGradientStopCollection,
            &pRadialGradientBrush
        );

        // 绘制发光效果
        pRenderTarget->FillEllipse(
            D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), radius, radius),
            pRadialGradientBrush
        );

        // 释放资源
        pGradientStopCollection->Release();
        pRadialGradientBrush->Release();
    }
}

void MiniAppUI::DrawGeometry(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush,  std::vector<D2D1_POINT_2F>& geomPoints,bool fill)
{
    if (geomPoints.size() < 3) return;

    // 创建路径几何对象
    ID2D1PathGeometry* pPathGeometry = nullptr;
    ID2D1Factory* pFactory = nullptr;
    pRenderTarget->GetFactory(&pFactory);

    HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
    if (SUCCEEDED(hr))
    {
        ID2D1GeometrySink* pSink = nullptr;
        hr = pPathGeometry->Open(&pSink);

        if (SUCCEEDED(hr))
        {
            // 开始绘制路径
            pSink->BeginFigure(geomPoints[0], D2D1_FIGURE_BEGIN_FILLED);

            for (int i=1;i<geomPoints.size();++i)
            {
                pSink->AddLine(geomPoints[i]);
            }

            // 完成图形
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            pSink->Close();
            pSink->Release();

            // 填充路径
            if (fill)
            {
                pRenderTarget->FillGeometry(pPathGeometry, brush);
            }
            else {
                pRenderTarget->DrawGeometry(pPathGeometry, brush, 2.0f);
            }
        }

        pPathGeometry->Release();
    }

    SafeRelease(&pFactory);
}

void MiniAppUI::DrawSin(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush, D2D1_POINT_2F centerPoint, float width, float height,float amplitude,float frequency)
{// 检查参数是否有效
	if (!pRenderTarget || !brush)
		return;

	// 创建路径几何对象
	ID2D1PathGeometry* pPathGeometry = nullptr;
	ID2D1Factory* pFactory = nullptr;
	pRenderTarget->GetFactory(&pFactory);

	HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);

	// 构建第一条波浪线路径
	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink* pSink = nullptr;
		hr = pPathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			// 设置起点（从左边开始）
			float startX = centerPoint.x - width / 2;
			float centerY = centerPoint.y;

			pSink->BeginFigure(D2D1::Point2F(startX, centerY), D2D1_FIGURE_BEGIN_FILLED);

			// 生成波浪线点
			const int numPoints = 100;
			float stepX = width / numPoints;

			// 正弦波参数
			for (int i = 1; i <= numPoints; ++i)
			{
				float x = startX + i * stepX;
				float y = centerY - amplitude * sinf(frequency * (x - startX)/width * 3.14159f);
				pSink->AddLine(D2D1::Point2F(x, y));
			}

			pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			hr = pSink->Close();
			pSink->Release();
		}
	}

	// 绘制波浪线
	if (SUCCEEDED(hr))
	{
		pRenderTarget->DrawGeometry(pPathGeometry, brush, 2.0f);
	}

	// 释放资源
    SafeRelease(&pPathGeometry);
    SafeRelease(&pFactory);
}

void MiniAppUI::DrawCos(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush, D2D1_POINT_2F centerPoint, float width, float height, float amplitude, float frequency)
{// 检查参数是否有效
	if (!pRenderTarget || !brush)
		return;

	// 创建路径几何对象
	ID2D1PathGeometry* pPathGeometry = nullptr;
	ID2D1Factory* pFactory = nullptr;
	pRenderTarget->GetFactory(&pFactory);

	HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);

	// 构建第一条波浪线路径
	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink* pSink = nullptr;
		hr = pPathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			// 设置起点（从左边开始）
			float startX = centerPoint.x - width / 2;
			float centerY = centerPoint.y;

			pSink->BeginFigure(D2D1::Point2F(startX, centerY-amplitude), D2D1_FIGURE_BEGIN_FILLED);

			// 生成波浪线点
			const int numPoints = 100;
			float stepX = width / numPoints;

			// 正弦波参数
			for (int i = 1; i <= numPoints; ++i)
			{
				float x = startX + i * stepX;
				float y = centerY - amplitude * cosf(frequency * (x - startX) / width * 3.14159f);
				pSink->AddLine(D2D1::Point2F(x, y));
			}

			pSink->EndFigure(D2D1_FIGURE_END_OPEN);
			hr = pSink->Close();
			pSink->Release();
		}
	}

	// 绘制波浪线
	if (SUCCEEDED(hr))
	{
		pRenderTarget->DrawGeometry(pPathGeometry, brush, 2.0f);
	}

	// 释放资源
	SafeRelease(&pPathGeometry);
	SafeRelease(&pFactory);
}

void MiniAppUI::DrawControlBtns(ID2D1RenderTarget* pRenderTarget)
{
    //绘制各个按钮
    int marginLeft = 5;
    int startX = marginLeft + m_BtnCircleRadius;
    int centerY = (rcClient.top + rcClient.bottom) / 2;

    ButtonType displayModeBtnType;
    switch (m_displayMode)
    {
    case SpectumMode::Cube:
		displayModeBtnType = ButtonType::CubeMode;
        break;
	case SpectumMode::CubeDrop:
		displayModeBtnType = ButtonType::CubeDropMode;
        break;
	case SpectumMode::Wave:
		displayModeBtnType = ButtonType::WaveMode;
        break;
	case SpectumMode::WaveDrop:
		displayModeBtnType = ButtonType::WaveDropMode;
        break;
	case SpectumMode::DoubleWave:
		displayModeBtnType = ButtonType::DoubleWaveMode;
        break;
    default:
        break;
    }

    ButtonType controlModeBtnTyp = ButtonType::Play;
    if (m_controlMode == ControlMode::Pause)
    {
        controlModeBtnTyp = ButtonType::Pause;
    }
    else if (m_controlMode == ControlMode::Stop)
    {
        controlModeBtnTyp = ButtonType::Stop;
    }
    //创建各个btn
    if (controlBtns.size() == 0)
    {
        controlBtns.push_back(ControlButton(displayModeBtnType, startX, centerY, 2 * m_BtnRadius));
        controlBtns.push_back(ControlButton(controlModeBtnTyp, startX + m_BtnCircleRadius - 7 + m_BtnSpacing +  m_BtnRadius, centerY, 2 * m_BtnRadius));
        controlBtns.push_back(ControlButton(ButtonType::Close, startX + m_BtnCircleRadius - 7 + (m_BtnSpacing + m_BtnRadius) * 2, centerY, 2 * m_BtnRadius));
    }
    else {
        //更新类型
        controlBtns[0].type = displayModeBtnType;
        controlBtns[1].type = controlModeBtnTyp;
    }
    for (auto& oneControlBtn : controlBtns)
    {
        DrawControlButton(pRenderTarget, oneControlBtn);
    }
}

D2D1::ColorF MiniAppUI::colorConvert(int r, int g, int b, int a) {
    return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}
