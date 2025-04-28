//#include "AppUI.h"
//
//#include <windowsx.h>
//#include <filesystem>
//#include <algorithm>
//
//AppUI::AppUI(HWND parent,int width, int height, int cornerRadius )
//    : m_hwnd(nullptr), m_parent(parent), m_cornerRadius(cornerRadius),
//    m_bgColor(D2D1::ColorF(0.2f, 0.2f, 0.2f, 0.6f)),
//    m_borderColor(D2D1::ColorF(0.27f, 0.51f, 0.78f, 1.0f)),
//    m_borderWidth(1.5f),
//    m_pD2DFactory(nullptr),
//    m_pRenderTarget(nullptr),
//    m_pBgBrush(nullptr),
//    m_pBorderBrush(nullptr) {
//    // Initialize Direct2D factory
//    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
//
//    // Register window class
//    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
//    wc.style = CS_HREDRAW | CS_VREDRAW| CS_DBLCLKS;
//    wc.lpfnWndProc = AppUI::WndProc;
//    wc.hInstance = GetModuleHandle(nullptr);
//    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
//    wc.lpszClassName = "RoundedWindowD2DClass";
//    if (!RegisterClassEx(&wc)) return ;
//
//    //计算位置
//    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
//    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
//
//    // Create layered window
//    int windowInitX = screenWidth - width-22; int windowInitY = screenHeight - 2.3 * height;
//    m_hwnd = CreateWindowEx(
//        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
//        "RoundedWindowD2DClass",
//        "Rounded Window D2D",
//        WS_POPUP,
//        screenWidth-2*width , screenHeight-1.5*height,
//        width, height,
//        m_parent,
//        nullptr,
//        GetModuleHandle(nullptr),
//        this
//    );
//    if (!m_hwnd) return ;
//
//    //初始化一些位置
//    actualWindowPos = POINT{ (LONG)windowInitX, (LONG)windowInitY };//初始化位置设置
//    lastMousePos = POINT();
//	//启动监听
//	player = new Player(m_hwnd, tracks);
//    //设置一些更新函数
//    player->setPlayIndexChangeCallback([this](int index,float trackLength) {
//        this->m_trackLength = trackLength;
//        this->PlayTrack(index);
//        });
//    player->setPlayTimeUpdateCallback([&playingPercentRef=m_playingPercent](float currentTime,float percent) {
//        playingPercentRef = percent;
//        });
//
//
//	spectumTimer = new SpectumTimer(player);
//	spectumTimer->Start();
//
//    HRESULT hr = CreateDeviceResources();
//    if (FAILED(hr)) return;
//}
//
//AppUI::~AppUI() {
//    Close();
//    DiscardDeviceResources();
//	SafeRelease(&m_pD2DFactory);
//	if (spectumTimer) {
//		spectumTimer->Stop();
//		delete spectumTimer;
//	}
//	if (player) delete player;
//}
//
//bool AppUI::Show() {
//    // Initial render
//    UpdateWindowAppearance();
//    ShowWindow(m_hwnd, SW_SHOW);
//    return true;
//}
//
//void AppUI::Close() {
//    if (m_hwnd) {
//        DestroyWindow(m_hwnd);
//        m_hwnd = nullptr;
//    }
//}
//
//void AppUI::SetBackgroundColor(D2D1_COLOR_F color) {
//    m_bgColor = color;
//    if (m_pBgBrush) {
//        m_pBgBrush->SetColor(color);
//    }
//    if (m_hwnd) UpdateWindowAppearance();
//}
//
//void AppUI::SetBorderColor(D2D1_COLOR_F color, float width) {
//    m_borderColor = color;
//    m_borderWidth = width;
//    if (m_pBorderBrush) {
//        m_pBorderBrush->SetColor(color);
//    }
//    if (m_hwnd) UpdateWindowAppearance();
//}
//
//void AppUI::Resize(int width, int height) {
//    if (m_hwnd) {
//        SetWindowPos(m_hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
//        if (m_pRenderTarget) {
//            m_pRenderTarget->Resize(D2D1::SizeU(width, height));
//        }
//        UpdateWindowAppearance();
//    }
//}
//
//LRESULT CALLBACK AppUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    if (msg == WM_NCCREATE) {
//        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
//        AppUI* pThis = reinterpret_cast<AppUI*>(pCreate->lpCreateParams);
//        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
//    }
//
//    AppUI* pThis = reinterpret_cast<AppUI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
//    return pThis ? pThis->HandleMessage(hwnd, msg, wParam, lParam) : DefWindowProc(hwnd, msg, wParam, lParam);
//}
//
//LRESULT AppUI::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    switch (msg) {
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//
//    case WM_LBUTTONDOWN: {
//        lastMousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
//        onWMLButtonDown(hwnd);
//        return 0;
//    }
//    case WM_LBUTTONUP: {
//        if (m_scrollThumbState==State::PRESSED) {
//            m_scrollThumbState = State::NONE;
//            ReleaseCapture();
//        }
//        if (m_timeLineBtnState==State::PRESSED)
//		{
//            m_timeLineBtnState = State::NONE;
//			ReleaseCapture();
//        }
//        return 0;
//    }
//    case WM_LBUTTONDBLCLK: {
//        //双击
//        lastMousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
//        onWMLBUTTONDBLCLK(hwnd);
//    }
//    case WM_PAINT:
//        UpdateWindowAppearance();
//        break;
//    case WM_MOVING: {
//        RECT* pRect = reinterpret_cast<RECT*>(lParam);
//        actualWindowPos.x = pRect->left;
//        actualWindowPos.y = pRect->top;
//        return TRUE;
//    }
//    case WM_MOUSEMOVE: {
//		lastMousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
//        onWMMouseMove(hwnd);
//        return 0;
//    }
//    case WM_MOUSELEAVE: {
//        onWMMouseLeave(hwnd);
//        return 0;
//    }
//
//    case WM_MOUSEWHEEL: {
//        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
//        ScrollTo(m_scrollPos - delta / 3);
//        return 0;
//    }
//    case WM_SIZE:
//        if (m_pRenderTarget) {
//            RECT rc;
//            GetClientRect(m_hwnd, &rc);
//            m_pRenderTarget->Resize(D2D1::SizeU(rc.right, rc.bottom));
//            UpdateWindowAppearance();
//        }
//        break;
//
//    case WM_NCHITTEST: {
//        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
//        ScreenToClient(hwnd, &pt);
//
//        RECT closeBtnRect = {
//            rcClient.right - m_closeBtnHeight - 10,
//            (m_titleBarHeight - m_closeBtnHeight) / 2,
//            rcClient.right - 10,
//            (m_titleBarHeight + m_closeBtnHeight) / 2
//        };
//        if (PtInRect(&closeBtnRect, pt)) {
//            return HTCLIENT;
//        }
//
//        if (pt.y <= m_titleBarHeight) {
//            return HTCAPTION;
//        }
//
//        return HTCLIENT;
//    }
//    default:
//        return DefWindowProc(hwnd, msg, wParam, lParam);
//    }
//    return 0;
//}
//
//HRESULT AppUI::CreateDeviceResources() {
//    HRESULT hr = S_OK;
//
//    if (!m_pRenderTarget) {
//        RECT rc;
//        GetClientRect(m_hwnd, &rc);
//
//        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
//
//        // Create a Direct2D render target
//        hr = m_pD2DFactory->CreateHwndRenderTarget(
//            D2D1::RenderTargetProperties(),
//            D2D1::HwndRenderTargetProperties(m_hwnd, size),
//            &m_pRenderTarget
//        );
//
//        if (SUCCEEDED(hr)) {
//            hr = m_pRenderTarget->CreateSolidColorBrush(m_bgColor, &m_pBgBrush);
//        }
//
//        if (SUCCEEDED(hr)) {
//            hr = m_pRenderTarget->CreateSolidColorBrush(m_borderColor, &m_pBorderBrush);
//        }
//
//        //创建内容对象
//        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
//            D2D1_RENDER_TARGET_TYPE_DEFAULT,
//            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
//            0, 0
//        );
//
//        hr = m_pD2DFactory->CreateDCRenderTarget(&props, &m_pDCRenderTarget);
//        m_pDCRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
//
//
//        //titlebar
//        // 创建DirectWrite工厂
//        hr = DWriteCreateFactory(
//            DWRITE_FACTORY_TYPE_SHARED,
//            __uuidof(IDWriteFactory),
//            reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
//        );
//
//        //创建titlebar的东西
//        if (SUCCEEDED(hr)) {
//            // 创建文本格式
//            hr = m_pDWriteFactory->CreateTextFormat(
//                L"微软雅黑,SimSun, Microsoft YaHei, Arial",                // 字体家族名
//                nullptr,                   // 字体集合(nullptr表示系统默认)
//                DWRITE_FONT_WEIGHT_SEMI_BOLD, // 字体粗细
//                DWRITE_FONT_STYLE_NORMAL,  // 字体样式
//                DWRITE_FONT_STRETCH_NORMAL,// 字体拉伸
//                18.0f,                     // 字体大小
//                L"zh-cn",                  // 区域设置
//                &m_titleBarTextFormat
//            );
//            if (SUCCEEDED(hr)) {
//                m_titleBarTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
//                m_titleBarTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
//            }
//            if (SUCCEEDED(hr)) {
//                // 创建文字画刷
//                hr = m_pDCRenderTarget->CreateSolidColorBrush(m_titleBarTextColor, &m_tileBarTextBrush);
//            }
//        }
//
//        //创建列表的
//        if (SUCCEEDED(hr)) {
//            // 创建文本格式
//            hr = m_pDWriteFactory->CreateTextFormat(
//                L"微软雅黑,SimSun, Microsoft YaHei, Arial",                // 字体家族名
//                nullptr,                   // 字体集合(nullptr表示系统默认)
//                DWRITE_FONT_WEIGHT_NORMAL, // 字体粗细
//                DWRITE_FONT_STYLE_NORMAL,  // 字体样式
//                DWRITE_FONT_STRETCH_NORMAL,// 字体拉伸
//                m_trackTextSize,                     // 字体大小
//                L"zh-cn",                  // 区域设置
//                &m_trackTextFormat
//            );
//            if (SUCCEEDED(hr)) {
//                m_trackTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
//                m_trackTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
//            }
//
//            hr = m_pDWriteFactory->CreateTextFormat(
//                L"微软雅黑",                // 字体家族名
//                nullptr,                   // 字体集合(nullptr表示系统默认)
//                DWRITE_FONT_WEIGHT_NORMAL, // 字体粗细
//                DWRITE_FONT_STYLE_NORMAL,  // 字体样式
//                DWRITE_FONT_STRETCH_NORMAL,// 字体拉伸
//                m_trackTextSize,                     // 字体大小
//                L"zh-cn",                  // 区域设置
//                &m_trackSecondaryTextFormat
//            );
//            if (SUCCEEDED(hr)) {
//                m_trackSecondaryTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
//                m_trackSecondaryTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
//            }
//
//            if (SUCCEEDED(hr)) {
//                // 创建文字画刷
//                hr = m_pDCRenderTarget->CreateSolidColorBrush(colorTrackBg, &m_trackBgBrush);
//                hr = m_pDCRenderTarget->CreateSolidColorBrush(colorTrackText, &m_trackTextBrush);
//                hr = m_pDCRenderTarget->CreateSolidColorBrush(colorTrackSecondaryText, &m_trackSeparetorBrush);
//            }
//        }
//
//        //创建时间线文字
//		if (SUCCEEDED(hr)) {
//			// 创建文本格式
//			hr = m_pDWriteFactory->CreateTextFormat(
//				L"微软雅黑,SimSun, Microsoft YaHei, Arial",                // 字体家族名
//				nullptr,                   // 字体集合(nullptr表示系统默认)
//				DWRITE_FONT_WEIGHT_SEMI_BOLD, // 字体粗细
//				DWRITE_FONT_STYLE_NORMAL,  // 字体样式
//				DWRITE_FONT_STRETCH_NORMAL,// 字体拉伸
//				12.0f,                     // 字体大小
//				L"zh-cn",                  // 区域设置
//				&m_timeLineTextFormat
//			);
//			if (SUCCEEDED(hr)) {
//                m_timeLineTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
//                m_timeLineTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
//			}
//		}
//    }
//
//    return hr;
//}
//
//void AppUI::DiscardDeviceResources() {
//    SafeRelease(&m_pRenderTarget);
//    SafeRelease(&m_pBgBrush);
//    SafeRelease(&m_pBorderBrush);
//    SafeRelease(&m_pDCRenderTarget);
//    SafeRelease(&m_tileBarTextBrush);
//    SafeRelease(&m_titleBarTextFormat);
//
//    SafeRelease(&m_trackTextFormat);
//    SafeRelease(&m_trackBgBrush);
//    SafeRelease(&m_trackTextBrush);
//    SafeRelease(&m_trackSeparetorBrush);
//    SafeRelease(&m_trackSecondaryTextFormat);
//
//    SafeRelease(&m_timeLineTextFormat);
//}
//
//void AppUI::UpdateScrollInfo() {
//    int contentHeight = tracks.size() * m_trackHeight;
//    m_scrollMax = max(0, contentHeight - (rcClient.bottom - m_titleBarHeight - m_controlPanelHeigh));
//    m_scrollPos = min(m_scrollPos, m_scrollMax);
//
//    float visibleRatio = (float)(rcClient.bottom - m_titleBarHeight - m_controlPanelHeigh) / contentHeight;
//    m_scrollThrumbHeight = max(30, (int)((rcClient.bottom - m_titleBarHeight - m_controlPanelHeigh) * visibleRatio));
//    rcScrollTrack = rcClient;
//    rcScrollTrack.left = rcClient.right - m_scrollWidth;
//    rcScrollTrack.top += m_titleBarHeight;
//    rcScrollTrack.bottom -= m_controlPanelHeigh;
//    UpdateThumbPosition();
//}
//
//void AppUI::UpdateThumbPosition() {
//    if (m_scrollMax > 0) {
//        float thumbPosRatio = (float)m_scrollPos / m_scrollMax;
//        int thumbY = (int)(thumbPosRatio * (rcScrollTrack.bottom - m_scrollThrumbHeight - m_titleBarHeight));
//
//        rcScrollThrumb = {
//            rcScrollTrack.left,
//            rcScrollTrack.top + thumbY,
//            rcScrollTrack.right,
//            rcScrollTrack.top + thumbY + m_scrollThrumbHeight
//        };
//    }
//    else {
//        rcScrollThrumb = { 0,0,0,0 };
//    }
//}
//void AppUI::ScrollTo(int newPos) {
//    newPos = max(0, min(m_scrollMax, newPos));
//    if (newPos != m_scrollPos) {
//        m_scrollPos = newPos;
//        UpdateThumbPosition();
//
//        RECT rcInvalidate = rcClient;
//        rcInvalidate.top = m_titleBarHeight;
//        rcInvalidate.bottom -= m_controlPanelHeigh;
//        InvalidateRect(m_hwnd, &rcInvalidate, FALSE);
//        UpdateWindow(m_hwnd);
//    }
//}
//
//std::wstring AppUI::CalcuTimeLineStr(int timeSecond)
//{
//    int h=0, m=0, s = 0;
//    h = timeSecond / 3600;
//    m = (timeSecond % 3600)/60;
//    s = (timeSecond % 3600) % 60;
//
//	std::wstring result = L"";
//    if (h>0&&h<10)
//    {
//        result=L"0";
//    }
//    if (h >0) {
//
//		result += (std::to_wstring(h) + L":");
//    }
//    if (m<10)
//    {
//        result += L"0";
//    }
//	result += (std::to_wstring(m) + L":");
//	if (s < 10)
//	{
//		result += L"0";
//	}
//	result += (std::to_wstring(s));
//  
//    return result;
//}
//
//void AppUI::SwitchDisplayMode()
//{
//	// 转换为整数进行计算
//	int next = static_cast<int>(m_displayMode) + 1;
//
//	// 如果超过枚举值数量则循环回到第一个
//	if (next >= static_cast<int>(SpectumMode::COUNT)) {
//		next = 0;
//	}
//
//	// 转换回枚举类型
//    m_displayMode = static_cast<SpectumMode>(next);
//
//}
//
//void AppUI::PlayTrack(int index)
//{
//    //这个函数只负责更新界面    
//    if (m_playingIndex == -1) m_playingIndex = 0;
//
//	if (index >= 0 && index < tracks.size()) {
//		tracks[m_playingIndex].state = State::NONE;
//        m_playingIndex = index;
//        m_isPlaying = true;
//        tracks[index].state = State::SELECTED;
//	}
//}
//
//void AppUI::PausePlay()
//{
//    m_isPlaying = false;
//	InvalidateRect(m_hwnd, &rcControlPanel, FALSE);
//	// Here you would pause the actual audio playback
//	player->PauseSong();
//}
//
//void AppUI::ResumePlay()
//{
//    m_isPlaying = true;
//    if (m_playingIndex == -1) m_playingIndex = 0;
//	InvalidateRect(m_hwnd, &rcControlPanel, FALSE);
//	// Here you would resume the actual audio playback
//	if (player->isPlaying()) {
//		player->PauseResumSong();
//	}
//	else {
//        m_playingIndex = 0;
//        player->PlaySong(m_playingIndex, m_trackLength);
//	}
//}
//
//void AppUI::PlayPrevious()
//{
//	if (m_playingIndex > 0) {
//		PlayTrack(m_playingIndex - 1);
//	}
//	else if (!tracks.empty()) {
//		// 直接播放最后一首
//		PlayTrack(tracks.size() - 1);
//	}
//	player->PlaySong(m_playingIndex, m_trackLength);
//	m_playingPercent = 0.0f;
//}
//
//void AppUI::PlayNext()
//{
//	if (m_playingIndex < tracks.size() - 1) {
//		PlayTrack(m_playingIndex + 1);
//	}
//	else if (!tracks.empty()) {
//		// 播放第一首
//		PlayTrack(0);
//	}
//	player->PlaySong(m_playingIndex, m_trackLength);
//    m_playingPercent = 0.0f;
//}
//
//void AppUI::AddMusicFiles()
//{
//    // Open file dialog to select music files
//    OPENFILENAMEW ofn;
//    wchar_t szFile[4028] = { 0 };
//
//    ZeroMemory(&ofn, sizeof(ofn));
//    ofn.lStructSize = sizeof(ofn);
//    ofn.hwndOwner = m_hwnd;
//    ofn.lpstrFile = szFile;
//    ofn.nMaxFile = sizeof(szFile);
//    ofn.lpstrFilter = L"Music Files\0*.mp3;*.wav;*.flac;*.m4a\0All Files\0*.*\0";
//    ofn.nFilterIndex = 1;
//    ofn.lpstrFileTitle = NULL;
//    ofn.nMaxFileTitle = 0;
//    ofn.lpstrInitialDir = NULL;
//    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
//
//    if (GetOpenFileNameW(&ofn)) {
//        // For this example, we'll just add a dummy track
//        // In a real application, you would parse the files and add them
//        // 第一个字符串是目录路径
//        const wchar_t* p = ofn.lpstrFile;
//        std::wstring directory = p;
//        p += directory.length() + 1;  // 移动到第一个文件名
//
//        // 如果只选了一个文件，lpstrFile 直接是完整路径
//        if (*p == 0) {
//            std::filesystem::path selectdFilePath(directory);			;
//            auto it=std::find_if(tracks.begin(), tracks.end(), [&directory](SongInfo oneInfo) {
//                return oneInfo.filePath == directory;
//            });
//            if (it==tracks.end())
//            {
//                tracks.push_back(SongInfo(selectdFilePath.stem().wstring(), L"未知", directory));
//            }
//        }
//        else {
//            // 遍历所有选中的文件
//            while (*p) {
//                std::wstring filename = p;
//                std::wstring oneSelectdFilePathStr = directory + L"\\" + filename;
//                std::filesystem::path selectdFilePath(oneSelectdFilePathStr);
//
//                auto it = std::find_if(tracks.begin(), tracks.end(), [&oneSelectdFilePathStr](SongInfo oneInfo) {
//                    return oneInfo.filePath == oneSelectdFilePathStr;
//                    });
//                if (it == tracks.end())
//                {
//                    tracks.push_back(SongInfo(selectdFilePath.stem().wstring(), L"未知", oneSelectdFilePathStr));
//                }
//
//                p += filename.length() + 1;  // 移动到下一个文件名
//            }
//        }
//
//        // Update scroll information since we added new tracks
//        m_scrollMax = max(0, tracks.size() * m_trackHeight - (rcClient.bottom - m_titleBarHeight - m_controlPanelHeigh));
//        UpdateScrollInfo();
//
//        // Force redraw
//        InvalidateRect(m_hwnd, NULL, FALSE);
//    }
//}
//
//
//void AppUI::onWMMouseMove(HWND hwnd)
//{
//    if (!isMouseTracking) {
//        TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_HOVER, hwnd, HOVER_DEFAULT };
//        TrackMouseEvent(&tme);
//        isMouseTracking = true;
//    }
//
//    onMouseMove_TittleBar(hwnd);
//    onMouseMove_ListAndScroll(hwnd);
//    mouseMove_ControlPanel(hwnd);
//    onMouseMove_TimeLine(hwnd);
//}
//
//void AppUI::onMouseMove_TittleBar(HWND hwnd)
//{
//    RECT closeBtnRect = {
//        rcClient.right - m_closeBtnHeight - 10,
//        (m_titleBarHeight - m_closeBtnHeight) / 2,
//        rcClient.right - 10,
//        (m_titleBarHeight + m_closeBtnHeight) / 2
//    };
//    bool newCloseBtnHovered = PtInRect(&closeBtnRect, lastMousePos);
//    if (newCloseBtnHovered != closeBtnHovered) {
//        closeBtnHovered = newCloseBtnHovered;
//        InvalidateRect(hwnd, &closeBtnRect, FALSE);
//    }
//}
//
//void AppUI::onMouseMove_ListAndScroll(HWND hwnd) {
//    if (State::PRESSED == m_scrollThumbState) {
//        int deltaY = lastMousePos.y - m_scrollDragStartY;
//        int newThumbY = m_scrollInitialPos + deltaY;
//        newThumbY = max(0, min(rcScrollTrack.bottom - m_scrollThrumbHeight, newThumbY));
//
//        float ratio = (float)newThumbY / (rcScrollTrack.bottom - m_scrollThrumbHeight);
//        ScrollTo((int)(ratio*m_scrollMax));
//    }else
//    {
//        mouseMove_List(hwnd);
//    }
//}
//void AppUI::mouseMove_List(HWND hwnd) {
//    bool newThumbHovered = PtInRect(&rcScrollThrumb, lastMousePos);
//    if (!newThumbHovered&&m_scrollThumbState!=State::NONE)
//    {
//        m_scrollThumbState = State::NONE;
//        InvalidateRect(hwnd, &rcScrollTrack, FALSE);
//    }
//    else if (newThumbHovered&&m_scrollThumbState == State::NONE)
//    {
//        m_scrollThumbState = State::HOVERED;
//        InvalidateRect(hwnd, &rcScrollTrack, FALSE);
//    }
//
//    int hoverIndex = (lastMousePos.y - m_titleBarHeight + m_scrollPos) / m_trackHeight;
//    bool needTrackRepaint = false;
//    for (size_t i=0;i<tracks.size();++i)
//    {
//        bool shouldHover = (i == hoverIndex) && hoverIndex > -1 && hoverIndex < tracks.size() && !PtInRect(&rcScrollTrack, lastMousePos);
//        if (tracks[i].state==State::NONE&&shouldHover)
//        {
//            tracks[i].state = State::HOVERED;
//            needTrackRepaint = true;
//        }else if (tracks[i].state != State::NONE && !shouldHover)
//        {
//            tracks[i].state = State::NONE;
//            needTrackRepaint = true;
//        }
//    }
//    //修复selectedTrack
//    if (m_playingIndex>-1&&tracks.size()>-1)
//	{
//		tracks[m_playingIndex].state = State::SELECTED;
//    }
//
//    if (needTrackRepaint) {
//        RECT rcInvalidate = rcClient;
//        rcInvalidate.top = m_titleBarHeight;
//        rcInvalidate.bottom = rcControlPanel.top;
//        InvalidateRect(hwnd, &rcInvalidate, FALSE);
//    }
//}
//
//void AppUI::mouseMove_ControlPanel(HWND hwnd)
//{
//    if (!PtInRect(&rcControlPanel, lastMousePos)) return;
//    for (auto& oneControlBtn : controlBtns)
//    {
//        if (oneControlBtn.isInBtn(lastMousePos))
//        {
//            oneControlBtn.state = State::HOVERED;
//        }
//        else {
//            oneControlBtn.state = State::NONE;
//        }
//    }
//}
//
//void AppUI::onMouseMove_TimeLine(HWND hwnd)
//{
//    if (m_timeLineBtnState==State::PRESSED)
//    {
//		m_playingPercent = (lastMousePos.x - rcTimeLine.left) / (m_timeLineWidth * 1.0);
//        m_playingPercent = max(0.0f, min(m_playingPercent, 1.0f));
//        player->PlayAtTime(m_playingPercent * m_trackLength);
//    }
//    RECT checkTimeline{ rcTimeLine.left,rcTimeLine.top - m_timeLineHeight,rcTimeLine.right,rcTimeLine.bottom + m_timeLineHeight  };
//	if (PtInRect(&checkTimeline, lastMousePos)&& m_timeLineState != State::HOVERED) {
//		m_timeLineState = State::HOVERED;
//    }else if (!PtInRect(&checkTimeline, lastMousePos) && m_timeLineState == State::HOVERED)
//    {
//        m_timeLineState = State::NONE;
//    }
//}
//
//void AppUI::onWMMouseLeave(HWND hwnd)
//{
//    isMouseTracking = false;
//    bool needRepaint = false;
//
//    for (auto& track : tracks)
//    {
//        if (track.state!=State::NONE)
//        {
//            track.state = State::NONE;
//            needRepaint = true;
//        }
//    }
//    //修复selectedTrack
//	if (m_playingIndex > -1&&tracks.size()>-1) {
//		tracks[m_playingIndex].state = State::SELECTED;
//    }
//
//    if (m_scrollThumbState!=State::NONE) {
//        m_scrollThumbState = State::NONE;
//        needRepaint = true;
//    }
//
//    if (closeBtnHovered) {
//        closeBtnHovered = false;
//        needRepaint = true;
//    }
//
//    if (m_timeLineState!=State::NONE)
//    {
//        m_timeLineState = State::NONE;
//    }
//
//    if (needRepaint) {
//        InvalidateRect(hwnd, NULL, FALSE);
//    }
//}
//
//void AppUI::onWMLButtonDown(HWND hwnd)
//{
//    RECT closeBtnRect = {
//        rcClient.right - m_closeBtnHeight - 10,
//        (m_titleBarHeight - m_closeBtnHeight) / 2,
//        rcClient.right - 10,
//        (m_titleBarHeight + m_closeBtnHeight) / 2
//    };
//	RECT checkTimelineRC{ rcTimeLine.left,rcTimeLine.top - m_timeLineHeight,rcTimeLine.right,rcTimeLine.bottom + m_timeLineHeight };
//
//    if (PtInRect(&closeBtnRect, lastMousePos)) {
//        PostMessage(hwnd, WM_CLOSE, 0, 0);
//        return;
//    }
//    else if (PtInRect(&rcScrollThrumb,lastMousePos))
//    {
//        if (m_scrollThumbState != State::PRESSED)
//        {
//            m_scrollThumbState = State::PRESSED;
//            m_scrollDragStartY = lastMousePos.y;
//            m_scrollInitialPos = rcScrollThrumb.top;
//            SetCapture(hwnd);
//        }
//	}
//	else if (PtInRect(&checkTimelineRC, lastMousePos))
//	{
//        //这个要先于controlbtn的判断 不然不会生效
//		//如果是在按钮上
//		if (PtInRect(&rcTimeLineBtn, lastMousePos))
//		{
//			if (m_timeLineBtnState != State::PRESSED)
//			{
//				m_timeLineBtnState = State::PRESSED;
//				m_timeLineBtnPressedX = lastMousePos.x;
//				SetCapture(hwnd);
//            }
//		}
//		else {
//			//如果是在时间条上 则直接把按钮的位置放过去
//			m_playingPercent = (lastMousePos.x - rcTimeLine.left) / (m_timeLineWidth*1.0);
//			m_playingPercent = max(0.0f, min(m_playingPercent, 1.0f));
//            player->PlayAtTime(m_playingPercent * m_trackLength);
//		}
//	}
//    else if (PtInRect(&rcControlPanel, lastMousePos))
//    {
//        lbuttonDownControlBtn(hwnd);
//    }
//}
//
//void AppUI::lbuttonDownControlBtn(HWND hwnd)
//{
//    for (auto& oneControlBtn:controlBtns)
//    {
//        if (!oneControlBtn.isInBtn(lastMousePos)) continue;
//        //下面根据实际的按钮情况来处理
//        switch (oneControlBtn.type)
//        {
//            case ButtonType::CubeMode: case ButtonType::WaveMode:case ButtonType::ThrowMode: {
//                //这三个情况相同
//                // 转换为整数进行计算
//                int next = static_cast<int>(m_displayMode) + 1;
//
//                // 如果超过枚举值数量则循环回到第一个
//                if (next == static_cast<int>(SpectumMode::COUNT)) {
//                    next = 0;
//                }
//
//                // 转换回枚举类型
//                m_displayMode = static_cast<SpectumMode>(next);
//                player->RecordSystemSound();
//                break;
//            }
//            case ButtonType::Previous: {
//                //播放上一首
//                PlayPrevious();
//                break;
//            }
//            case ButtonType::Play: {
//				//播放
//				ResumePlay();
//                //切换状态
//                oneControlBtn.state = State::PRESSED;
//                break;
//            }
//            case ButtonType::Pause: {
//				//暂停
//				PausePlay();
//                //切换状态
//                oneControlBtn.state = State::PRESSED;
//                break;
//            }
//            case ButtonType::Next: {
//                //播放上一首
//                PlayNext();
//                break;
//            }
//            case ButtonType::AddFolder: {
//                //添加文件
//                AddMusicFiles();
//                break;
//            }
//        }
//    }
//}
//
//void AppUI::onDblClickTrack(int clickIndex)
//{
//    if (clickIndex<0 || clickIndex>tracks.size()||clickIndex==m_playingIndex) return;
//    tracks[m_playingIndex].state = State::NONE;
//    tracks[clickIndex].state = State::SELECTED;
//    m_playingIndex = clickIndex;
//    //播放歌曲
//	bool playOk = player->PlaySong(m_playingIndex, m_trackLength);
//	m_playingPercent = 0.0f;
//    if (playOk)
//	{
//		controlBtns[2].type = ButtonType::Pause;
//    }
//}
//
//void AppUI::onWMLBUTTONDBLCLK(HWND hwnd)
//{
//    if (m_titleBarHeight<lastMousePos.y&&lastMousePos.y<rcControlPanel.top&&lastMousePos.x<rcScrollTrack.left)
//    {
//        int clieckedIndex = (lastMousePos.y - m_titleBarHeight + m_scrollPos) / m_trackHeight;
//        onDblClickTrack(clieckedIndex);
//    }
//}
//
//void AppUI::UpdateWindowAppearance() {
//    if (!m_hwnd) return;
//
//    // Create memory DC and DIB section
//    HDC hdcScreen = GetDC(nullptr);
//    HDC hdcMem = CreateCompatibleDC(hdcScreen);
//
//    GetClientRect(m_hwnd, &rcClient);
//    UpdateScrollInfo();
//
//    BITMAPINFO bmi = { 0 };
//    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
//    bmi.bmiHeader.biWidth = rcClient.right;
//    bmi.bmiHeader.biHeight = -rcClient.bottom;
//    bmi.bmiHeader.biPlanes = 1;
//    bmi.bmiHeader.biBitCount = 32;
//    bmi.bmiHeader.biCompression = BI_RGB;
//
//    void* pBits;
//    HBITMAP hbmMem = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
//    SelectObject(hdcMem, hbmMem);
//
//    // Create and bind DC render target
//    HRESULT hr = m_pDCRenderTarget->BindDC(hdcMem, &rcClient);
//    //if (FAILED(hr)) return;
//    m_pDCRenderTarget->BeginDraw();
//    m_pDCRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
//    m_pDCRenderTarget->Clear(D2D1::ColorF(0, 0.0f));
//
//    // 绘制所有元素
//    DrawAll(m_pDCRenderTarget);
//
//    hr = m_pDCRenderTarget->EndDraw();
//
//    // Update layered window
//    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
//    POINT ptSrc = { 0, 0 };
//    SIZE size = { rcClient.right, rcClient.bottom };
//    POINT ptDst;
//    ptDst = { actualWindowPos.x, actualWindowPos.y };
//   
//    UpdateLayeredWindow(m_hwnd, nullptr, &ptDst, &size, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);
//
//    // Clean up
//    DeleteObject(hbmMem);
//    DeleteDC(hdcMem);
//    ReleaseDC(nullptr, hdcScreen);
//}
//
//void AppUI::DrawAll(ID2D1RenderTarget* pRenderTarget) {
//    DrawRoundedFrame(pRenderTarget);
//    DrawTitleBar(pRenderTarget);
//    DrawTracks(pRenderTarget);
//    DrawScrollBar(pRenderTarget);
//    DrawControlPanel(pRenderTarget);
//	DrawTimeline(pRenderTarget);
//}
//
//void AppUI::DrawRoundedFrame(ID2D1RenderTarget* pRenderTarget) {
//    float halfBorder = m_borderWidth / 2.0f;
//    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
//        D2D1::RectF(
//            halfBorder,
//            halfBorder,
//            static_cast<float>(rcClient .right) - halfBorder,
//            static_cast<float>(rcClient.bottom) - halfBorder
//        ),
//        static_cast<float>(m_cornerRadius),
//        static_cast<float>(m_cornerRadius)
//    );
//
//    // Create temporary brushes for this render target
//    ID2D1SolidColorBrush* pBgBrush = nullptr;
//    ID2D1SolidColorBrush* pBorderBrush = nullptr;
//    HRESULT hr;
//
//    if (FAILED(hr = pRenderTarget->CreateSolidColorBrush(m_bgColor, &pBgBrush))) {
//        // Handle error (log, return, etc.)
//        return;
//    }
//
//    if (FAILED(hr = pRenderTarget->CreateSolidColorBrush(m_borderColor, &pBorderBrush))) {
//        SafeRelease(&pBgBrush);
//        return;
//    }
//
//    // Perform the drawing
//    pRenderTarget->FillRoundedRectangle(roundedRect, pBgBrush);
//    pRenderTarget->DrawRoundedRectangle(roundedRect, pBorderBrush, m_borderWidth);
//
//    SafeRelease(&pBgBrush);
//    SafeRelease(&pBorderBrush);
//}
//
//void AppUI::DrawTopRoundedRect(ID2D1RenderTarget* pRenderTarget,
//    const D2D1_RECT_F& rect,
//    float topCornerRadius,
//    ID2D1Brush* pFillBrush )
//{
//    // 创建路径几何对象
//    ID2D1PathGeometry* pPathGeometry = nullptr;
//    ID2D1Factory* pFactory = nullptr;
//    pRenderTarget->GetFactory(&pFactory);
//
//    HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
//    if (SUCCEEDED(hr))
//    {
//        ID2D1GeometrySink* pSink = nullptr;
//        hr = pPathGeometry->Open(&pSink);
//
//        if (SUCCEEDED(hr))
//        {
//            // 计算各点位置
//            float left = rect.left;
//            float top = rect.top;
//            float right = rect.right;
//            float bottom = rect.bottom;
//
//            // 开始绘制路径
//            pSink->BeginFigure(D2D1::Point2F(left + topCornerRadius, top), D2D1_FIGURE_BEGIN_FILLED);
//
//            // 顶部左边圆角
//            pSink->AddArc(D2D1::ArcSegment(
//                D2D1::Point2F(left, top + topCornerRadius),
//                D2D1::SizeF(topCornerRadius, topCornerRadius),
//                0.0f,
//                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
//                D2D1_ARC_SIZE_SMALL
//            ));
//
//            // 左边直线到底部
//            pSink->AddLine(D2D1::Point2F(left, bottom));
//
//            // 底部右边直线
//            pSink->AddLine(D2D1::Point2F(right, bottom));
//
//            // 右边直线到顶部
//            pSink->AddLine(D2D1::Point2F(right, top + topCornerRadius));
//
//            // 顶部右边圆角
//            pSink->AddArc(D2D1::ArcSegment(
//                D2D1::Point2F(right - topCornerRadius, top),
//                D2D1::SizeF(topCornerRadius, topCornerRadius),
//                0.0f,
//                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
//                D2D1_ARC_SIZE_SMALL
//            ));
//
//            // 完成图形
//            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
//            pSink->Close();
//            pSink->Release();
//
//            // 填充路径
//            pRenderTarget->FillGeometry(pPathGeometry, pFillBrush);
//        }
//
//        pPathGeometry->Release();
//    }
//
//    SafeRelease(&pFactory);
//}
//
//void AppUI::DrawBottomRoundedRect(ID2D1RenderTarget* pRenderTarget,
//    const D2D1_RECT_F& rect,
//    float bottomCornerRadius,
//    ID2D1Brush* pFillBrush)
//{
//    // 创建路径几何对象
//    ID2D1PathGeometry* pPathGeometry = nullptr;
//    ID2D1Factory* pFactory = nullptr;
//    pRenderTarget->GetFactory(&pFactory);
//
//    HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
//    if (SUCCEEDED(hr))
//    {
//        ID2D1GeometrySink* pSink = nullptr;
//        hr = pPathGeometry->Open(&pSink);
//
//        if (SUCCEEDED(hr))
//        {
//            // 计算各点位置
//            float left = rect.left;
//            float top = rect.top;
//            float right = rect.right;
//            float bottom = rect.bottom;
//
//            // 开始绘制路径
//            pSink->BeginFigure(D2D1::Point2F(left,top), D2D1_FIGURE_BEGIN_FILLED);
//            pSink->AddLine(D2D1::Point2F(left, bottom - bottomCornerRadius));
//            pSink->AddArc(D2D1::ArcSegment(
//                D2D1::Point2F(left+bottomCornerRadius,bottom),
//                D2D1::SizeF(bottomCornerRadius, bottomCornerRadius),
//                0.0f,
//                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
//                D2D1_ARC_SIZE_SMALL
//            ));
//            pSink->AddLine(D2D1::Point2F(right - bottomCornerRadius, bottom));
//            pSink->AddArc(D2D1::ArcSegment(
//                D2D1::Point2F(right, bottom-bottomCornerRadius),
//                D2D1::SizeF(bottomCornerRadius, bottomCornerRadius),
//                0.0f,
//                D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
//                D2D1_ARC_SIZE_SMALL
//            ));
//            pSink->AddLine(D2D1::Point2F(right,top));
//            // 完成图形
//            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
//            pSink->Close();
//            pSink->Release();
//
//            // 填充路径
//            pRenderTarget->FillGeometry(pPathGeometry, pFillBrush);
//        }
//
//        pPathGeometry->Release();
//    }
//
//    SafeRelease(&pFactory);
//}
//
//void AppUI::DrawControlButton(ID2D1RenderTarget* pRenderTarget, ControlButton& controlBtn)
//{
//    //这个按钮要实现鼠标放上去放大 并且叠加一层泛光效果
//    float scale = (controlBtn.state == State::HOVERED) ? 1.1f : 1.0f;
//    int adjustedSize = (int)(controlBtn.size * scale);
//
//    D2D1_COLOR_F btnColor=colorControlBtn;
//    if (controlBtn.state == State::HOVERED) {
//        btnColor = colorControlBtnHover;
//    }else if (controlBtn.state==State::PRESSED)
//    {
//        btnColor = colorControlBtnPressed;
//    }
//
// //   m_trackBgBrush->SetColor(btnColor);
//    //D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), adjustedSize, adjustedSize);
//    //pRenderTarget->FillEllipse(&ellipse, m_trackBgBrush);
//
//    //绘制图标 因为图标要比按钮范围小 所以再缩放下
//    float iconScale = 0.5f;
//    float iconSize = adjustedSize * iconScale;
//    float iconX = controlBtn.centerX - iconSize / 2;
//    float iconY = controlBtn.centerY - iconSize / 2;
//    m_trackBgBrush->SetColor(colorControlBtnIcon);
//
//    switch (controlBtn.type)
//    {
//    case ButtonType::Previous: {
//        float barWidth = iconSize / 6;
//        D2D1_RECT_F preBar = D2D1::RectF(iconX, iconY, iconX + barWidth, iconY + iconSize);
//        pRenderTarget->FillRectangle(&preBar, m_trackBgBrush);
//
//        std::vector<D2D1_POINT_2F> geomPoints = {
//            D2D1::Point2F(iconX+iconSize,iconY),
//            D2D1::Point2F(iconX,iconY+iconSize/2),
//            D2D1::Point2F(iconX + iconSize,iconY+iconSize),
//        };
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        DrawGeometry(pRenderTarget, m_trackBgBrush, geomPoints);
//        break;
//    }
//    case ButtonType::Play : {
//        m_trackBgBrush->SetColor(colorContolBtnPlayCicle);
//        D2D1_ELLIPSE fillCicle=D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), 1.5 * iconSize, 1.5 * iconSize);
//        pRenderTarget->FillEllipse(&fillCicle, m_trackBgBrush);
//
//        std::vector<D2D1_POINT_2F> geomPoints = {
//            D2D1::Point2F(iconX,iconY),
//            D2D1::Point2F(iconX+iconSize,iconY + iconSize / 2),
//            D2D1::Point2F(iconX ,iconY + iconSize),
//        };
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        DrawGeometry(pRenderTarget, m_trackBgBrush, geomPoints);
//
//        //因为现在播放按钮有个外部圆圈 所以其RECT需要更新
//        int newRcRadius = 1.5 * iconSize;
//        controlBtn.rc = RECT{ controlBtn.centerX - newRcRadius,controlBtn.centerY - newRcRadius,controlBtn.centerX + newRcRadius,controlBtn.centerY + newRcRadius };
//        break;
//    }
//    case ButtonType::Pause: {
//        m_trackBgBrush->SetColor(colorContolBtnPlayCicle);
//        D2D1_ELLIPSE fillCicle = D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), 1.5 * iconSize, 1.5 * iconSize);
//        pRenderTarget->FillEllipse(&fillCicle, m_trackBgBrush);
//
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        float barWidth = iconSize / 3; float spacing = iconSize / 3;
//        D2D1_RECT_F rect1 = D2D1::RectF(iconX+1, iconY, iconX + barWidth, iconY + iconSize);
//        D2D1_RECT_F rect2 = D2D1::RectF(iconX+1+barWidth+spacing, iconY, iconX + 2*barWidth+spacing, iconY + iconSize);
//        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect2, m_trackBgBrush);
//
//        //因为现在暂停按钮有个外部圆圈 所以其RECT需要更新
//        int newRcRadius = 1.5 * iconSize;
//        controlBtn.rc = RECT{controlBtn.centerX-newRcRadius,controlBtn.centerY-newRcRadius,controlBtn.centerX+newRcRadius,controlBtn.centerY+newRcRadius};
//
//        break;
//    }
//    case ButtonType::Next: {
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        float barWidth = iconSize / 6;
//        D2D1_RECT_F rect1 = D2D1::RectF(iconX + iconSize - barWidth, iconY, iconX + iconSize , iconY + iconSize);
//        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
//
//        std::vector<D2D1_POINT_2F> geomPoints = {
//            D2D1::Point2F(iconX,iconY),
//            D2D1::Point2F(iconX + iconSize,iconY + iconSize / 2),
//            D2D1::Point2F(iconX ,iconY + iconSize),
//        };
//        DrawGeometry(pRenderTarget, m_trackBgBrush, geomPoints);
//        break;
//    }
//    case ButtonType::AddFolder: {
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        float offset = iconSize / 8;
//        float space = iconSize / 4;
//        pRenderTarget->DrawLine(D2D1::Point2F(iconX + offset, iconY), D2D1::Point2F(iconX + iconSize - offset, iconY), m_trackBgBrush,2.0f);
//
//        float leftUpX = iconX; float leftUpY = iconY + space;
//        float leftDownX = iconX + offset; float leftDownY = iconY + iconSize;
//        float rightUpX = iconX + iconSize; float rightUpY = iconY + space;
//        float righDownX = iconX + iconSize - offset; float rightDownY = iconY + iconSize;
//        std::vector<D2D1_POINT_2F> geomPoints = {
//            D2D1::Point2F(leftUpX,leftUpY),
//            D2D1::Point2F(leftDownX,leftDownY),
//            D2D1::Point2F(righDownX,rightDownY),
//            D2D1::Point2F(rightUpX,rightUpY),
//        };
//        DrawGeometry(pRenderTarget, m_trackBgBrush, geomPoints, false);
//
//
//        //再绘制一个十字
//        float crossRadius = 4;
//        pRenderTarget->DrawLine(D2D1::Point2F(iconX + iconSize / 2 - crossRadius, iconY + (space + iconSize) / 2), D2D1::Point2F(iconX + iconSize / 2 + crossRadius, iconY + (space + iconSize) / 2), m_trackBgBrush, 2.0f);
//        pRenderTarget->DrawLine(D2D1::Point2F(iconX + iconSize / 2, iconY + (space + iconSize) / 2 - crossRadius), D2D1::Point2F(iconX + iconSize / 2, iconY + (space + iconSize) / 2 + crossRadius), m_trackBgBrush, 2.0f);
//
//        break;
//    }
//    case ButtonType::CubeMode: {
//        float barWidth = iconSize / 8;
//        float spacing = iconSize / 6;
//
//        D2D1_RECT_F rect1 = D2D1::RectF(iconX, iconY + 2, iconX+barWidth, iconY+iconSize );
//        D2D1_RECT_F rect2 = D2D1::RectF(iconX + barWidth + spacing, iconY, iconX + barWidth + spacing+barWidth, iconY + iconSize);
//        D2D1_RECT_F rect3 = D2D1::RectF(iconX + (barWidth + spacing) * 2, iconY + 4, iconX + (barWidth + spacing) * 2+barWidth, iconY + iconSize);
//        D2D1_RECT_F rect4 = D2D1::RectF(iconX + (barWidth + spacing) * 3, iconY + 2, iconX + (barWidth + spacing) * 3+barWidth, iconY + iconSize);
//
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect2, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect3, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect4, m_trackBgBrush);
//        break;
//    }
//    case ButtonType::WaveMode: {
//        float barWidth = iconSize / 8;
//        float spacing = iconSize / 6;
//
//        D2D1_RECT_F rect1 = D2D1::RectF(iconX, iconY + 6, iconX+barWidth, iconY + iconSize);
//        D2D1_RECT_F rect2 = D2D1::RectF(iconX + barWidth + spacing, iconY + 8, iconX + barWidth + spacing+barWidth, iconY + iconSize);
//        D2D1_RECT_F rect3 = D2D1::RectF(iconX + (barWidth + spacing) * 2, iconY + 4, iconX + (barWidth + spacing) * 2+barWidth, iconY + iconSize);
//        D2D1_RECT_F rect4 = D2D1::RectF(iconX + (barWidth + spacing) * 3, iconY + 9, iconX + (barWidth + spacing) * 3+barWidth, iconY + iconSize);
//
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect2, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect3, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect4, m_trackBgBrush);
//
//        std::vector<D2D1_POINT_2F> geomPoints = {
//            D2D1::Point2F(iconX, iconY + 2),
//            D2D1::Point2F(iconX + barWidth + spacing, iconY + 4),
//            D2D1::Point2F(iconX + (barWidth + spacing) * 2, iconY),
//            D2D1::Point2F(iconX + (barWidth + spacing) * 3 + 2, iconY + 5),
//        };
//        for (int i=0;i<3;++i)
//        {
//            pRenderTarget->DrawLine(geomPoints[i], geomPoints[i + 1], m_trackBgBrush, 2.0f);
//        }
//        break;
//    }
//    case ButtonType::ThrowMode: {
//        float barWidth = iconSize / 8;
//        float spacing = iconSize / 6;
//
//        D2D1_RECT_F rect1 = D2D1::RectF(iconX, iconY, iconX+barWidth, iconY+iconSize - 2);
//        D2D1_RECT_F rect2 = D2D1::RectF(iconX + barWidth + spacing, iconY + 4, iconX + barWidth + spacing+barWidth, iconY+iconSize + 2);
//        D2D1_RECT_F rect3 = D2D1::RectF(iconX + (barWidth + spacing) * 2, iconY, iconX + (barWidth + spacing) * 2+barWidth, iconY+iconSize - 2);
//        D2D1_RECT_F rect4 = D2D1::RectF(iconX + (barWidth + spacing) * 3, iconY + 4, iconX + (barWidth + spacing) * 3+barWidth, iconY + iconSize + 2);
//
//        m_trackBgBrush->SetColor(colorControlBtnIcon);
//        pRenderTarget->FillRectangle(&rect1, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect2, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect3, m_trackBgBrush);
//        pRenderTarget->FillRectangle(&rect4, m_trackBgBrush);
//        break;
//    }
//    }
//    // 当按钮被hover时显示泛光效果
//    if (controlBtn.state==State::HOVERED) {
//        // 创建径向渐变笔刷以实现发光效果
//        ID2D1RadialGradientBrush* pRadialGradientBrush = nullptr;
//
//        // 定义渐变停止点
//        D2D1_GRADIENT_STOP gradientStops[2];
//        gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::White, 0.2f); // 中心半透明白色
//        gradientStops[0].position = 0.0f;
//        gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::White, 0.0f); // 边缘完全透明
//        gradientStops[1].position = 1.0f;
//
//        ID2D1GradientStopCollection* pGradientStopCollection = nullptr;
//        pRenderTarget->CreateGradientStopCollection(
//            gradientStops,
//            2,
//            D2D1_GAMMA_2_2,
//            D2D1_EXTEND_MODE_CLAMP,
//            &pGradientStopCollection
//        );
//
//        // 计算扩展尺寸
//        float expandedSize = adjustedSize * 1.2f;
//        float radius = expandedSize / 2.0f;
//
//        // 创建径向渐变笔刷
//        pRenderTarget->CreateRadialGradientBrush(
//            D2D1::RadialGradientBrushProperties(
//                D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), // 中心点
//                D2D1::Point2F(0, 0), // 偏移量
//                radius, radius),     // 半径
//            pGradientStopCollection,
//            &pRadialGradientBrush
//        );
//
//        // 绘制发光效果
//        pRenderTarget->FillEllipse(
//            D2D1::Ellipse(D2D1::Point2F(controlBtn.centerX, controlBtn.centerY), radius, radius),
//            pRadialGradientBrush
//        );
//
//        // 释放资源
//        pGradientStopCollection->Release();
//        pRadialGradientBrush->Release();
//    }
//}
//
//void AppUI::DrawGeometry(ID2D1RenderTarget* pRenderTarget, ID2D1SolidColorBrush* brush,  std::vector<D2D1_POINT_2F>& geomPoints,bool fill)
//{
//    if (geomPoints.size() < 3) return;
//
//    // 创建路径几何对象
//    ID2D1PathGeometry* pPathGeometry = nullptr;
//    ID2D1Factory* pFactory = nullptr;
//    pRenderTarget->GetFactory(&pFactory);
//
//    HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
//    if (SUCCEEDED(hr))
//    {
//        ID2D1GeometrySink* pSink = nullptr;
//        hr = pPathGeometry->Open(&pSink);
//
//        if (SUCCEEDED(hr))
//        {
//            // 开始绘制路径
//            pSink->BeginFigure(geomPoints[0], D2D1_FIGURE_BEGIN_FILLED);
//
//            for (int i=1;i<geomPoints.size();++i)
//            {
//                pSink->AddLine(geomPoints[i]);
//            }
//
//            // 完成图形
//            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
//            pSink->Close();
//            pSink->Release();
//
//            // 填充路径
//            if (fill)
//            {
//                pRenderTarget->FillGeometry(pPathGeometry, brush);
//            }
//            else {
//                pRenderTarget->DrawGeometry(pPathGeometry, brush, 2.0f);
//            }
//        }
//
//        pPathGeometry->Release();
//    }
//
//    SafeRelease(&pFactory);
//}
//
//void AppUI::DrawTimeline(ID2D1RenderTarget* pRenderTarget)
//{
//	//获取
//	int offset = (rcClient.right - m_timeLineWidth) / 2;
//	int offsetTop = 10;
//    int controlBtnCenterY = (controlBtns[2].rc.bottom + controlBtns[2].rc.top) / 2;
//    int maxBtnWidth = 1.1 * 0.5 * 1.5 * m_BtnSize;
//	rcTimeLine = { offset,controlBtnCenterY+maxBtnWidth+offsetTop,rcClient.right - offset,controlBtnCenterY + maxBtnWidth + offsetTop + m_timeLineHeight };
//
//	m_trackBgBrush->SetColor(colorTimeLineBg);
//	D2D1_ROUNDED_RECT timeLineRoundedRect = D2D1::RoundedRect(
//		D2D1::RectF(
//            rcTimeLine.left, rcTimeLine.top, rcTimeLine.right, rcTimeLine.bottom
//		),
//		static_cast<float>(m_timeLineHeight / 2),
//		static_cast<float>(m_timeLineHeight / 2)
//	);
//	pRenderTarget->FillRoundedRectangle(&timeLineRoundedRect, m_trackBgBrush);
//
//    //再绘制已经播放（拖动）的部分
//    D2D1_COLOR_F colorDrawTimeLineProgressed = colorTimeLineProgressed;
//    if (m_timeLineState==State::HOVERED)
//    {
//        colorDrawTimeLineProgressed = colorTimeLineProgressedHover;
//    }
//	m_trackBgBrush->SetColor(colorDrawTimeLineProgressed);
//	D2D1_ROUNDED_RECT timeLinePlayedRoundedRect = D2D1::RoundedRect(
//		D2D1::RectF(
//			rcTimeLine.left, rcTimeLine.top, rcTimeLine.left + m_playingPercent * m_timeLineWidth, rcTimeLine.bottom
//		),
//		static_cast<float>(m_timeLineHeight / 2),
//		static_cast<float>(m_timeLineHeight / 2)
//	);
//	pRenderTarget->FillRoundedRectangle(&timeLinePlayedRoundedRect, m_trackBgBrush);
//
//	D2D1_COLOR_F timeLineDrawColor = colorTimeLineBtn;
//	if (m_timeLineState == State::PRESSED) {
//		timeLineDrawColor = colorTimeLineBtnPressed;
//	}
//	else if (m_timeLineState == State::HOVERED) {
//		timeLineDrawColor = colorTimeLineBtnHover;
//	}
//	m_trackBgBrush->SetColor(timeLineDrawColor);
//
//    int centerX = rcTimeLine.left + m_playingPercent * m_timeLineWidth;
//    int centerY = (rcTimeLine.bottom + rcTimeLine.top) / 2;
//	rcTimeLineBtn = {
//        centerX -m_timeLineBtnRadius
//		,centerY - m_timeLineBtnRadius
//		,centerX + m_timeLineBtnRadius
//		,centerY + m_timeLineBtnRadius
//	};
//
//	pRenderTarget->FillEllipse(
//		&D2D1::Ellipse(
//			D2D1::Point2F(centerX,centerY )
//			, m_timeLineBtnRadius, m_timeLineBtnRadius)
//		, m_trackBgBrush);
//
//    //创建文字
//    std::wstring startText = CalcuTimeLineStr(int(m_playingPercent*m_trackLength));
//    std::wstring endText = CalcuTimeLineStr(int(m_trackLength));
//    
//	m_timeLineTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
//    m_trackTextBrush->SetColor(colorTimeLineText);
//	pRenderTarget->DrawText(
//        startText.c_str(),
//		static_cast<UINT32>(startText.length()),
//		m_timeLineTextFormat,
//        D2D1::RectF(rcTimeLine.left, rcTimeLine.bottom, rcTimeLine.left+60, rcTimeLine.bottom +20),
//        m_trackTextBrush
//	);
//
//	m_timeLineTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
//	pRenderTarget->DrawText(
//		endText.c_str(),
//		static_cast<UINT32>(endText.length()),
//		m_timeLineTextFormat,
//		D2D1::RectF(rcTimeLine.right-60, rcTimeLine.bottom, rcTimeLine.right, rcTimeLine.bottom + 20),
//		m_trackTextBrush
//	);
//
//}
//
//void AppUI::forceRedrew()
//{
//	InvalidateRect(m_hwnd, NULL, FALSE);
//}
//
//void AppUI::DrawTitleBar(ID2D1RenderTarget* pRenderTarget) {
//    // 创建画刷
//    ID2D1SolidColorBrush* pFillBrush = nullptr;
//    pRenderTarget->CreateSolidColorBrush(colorTitleBar, &pFillBrush);
//
//    // 定义矩形区域
//    D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f,rcClient.right, (float)m_titleBarHeight);
//    // 绘制顶部圆角、底部直角的矩形
//    DrawTopRoundedRect(pRenderTarget, rect, m_cornerRadius, pFillBrush);
//    //绘制矩形底部的一根分割线
//    ID2D1SolidColorBrush* pBottomLineBrush = nullptr;
//    HRESULT hr = pRenderTarget->CreateSolidColorBrush(colorTitleBarBottomLine, &pBottomLineBrush);
//    pRenderTarget->DrawLine(
//        D2D1::Point2F(0, m_titleBarHeight),
//        D2D1::Point2F(rcClient.right, m_titleBarHeight),
//        pBottomLineBrush, 2.0f
//    );
//
//    //绘制标题文字
//    if (!m_titleBarTextFormat || !m_tileBarTextBrush) return;
//
//    pRenderTarget->DrawText(
//        m_titleBarText.c_str(),
//        static_cast<UINT32>(m_titleBarText.length()),
//        m_titleBarTextFormat,
//        rect,
//        m_tileBarTextBrush
//    );
//
//    //绘制关闭按钮
//    RECT closeBtnRect = {
//        rcClient.right - m_closeBtnHeight - 10,
//        (m_titleBarHeight - m_closeBtnHeight) / 2,
//        rcClient.right - 10,
//        (m_titleBarHeight + m_closeBtnHeight) / 2
//    };
//    int centerX = (closeBtnRect.left + closeBtnRect.right) / 2;
//    int centerY = (closeBtnRect.top + closeBtnRect.bottom) / 2;
//    int crossSize = 6;
//
//
//    ID2D1SolidColorBrush* pFillBrushBtnBg = nullptr;
//    hr = pRenderTarget->CreateSolidColorBrush(closeBtnHovered ? colorCloseBtnHoverBG : colorTitleBar, &pFillBrushBtnBg);
//    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(centerX, centerY), m_closeBtnHeight * 0.7, m_closeBtnHeight * 0.7);
//    pRenderTarget->FillEllipse(&ellipse, pFillBrushBtnBg);
//
//    ID2D1SolidColorBrush* pBrush = nullptr;
//    hr = pRenderTarget->CreateSolidColorBrush(colorCloseBtnX, &pBrush);
//    pRenderTarget->DrawLine(
//        D2D1::Point2F(centerX - crossSize, centerY - crossSize),
//        D2D1::Point2F(centerX + crossSize, centerY + crossSize),
//        pBrush, 2.0f
//    );
//    pRenderTarget->DrawLine(
//        D2D1::Point2F(centerX + crossSize, centerY - crossSize),
//        D2D1::Point2F(centerX - crossSize, centerY + crossSize),
//        pBrush, 2.0f
//    );
//
//    // 释放画刷
//    SafeRelease(&pFillBrush);
//    SafeRelease(&pBottomLineBrush);
//    SafeRelease(&pBrush);
//    SafeRelease(&pFillBrushBtnBg);
//}
//void AppUI::DrawTracks(ID2D1RenderTarget* pRenderTarget) {
//    //先设置裁剪
//    D2D1_RECT_F clipRect = D2D1::RectF(
//        rcClient.left,                     // 左边界
//        rcClient.top+m_titleBarHeight,                     // 上边界
//        rcClient.right-m_scrollWidth, // 右边界（避开滚动条）
//        rcClient.bottom-m_controlPanelHeigh                 // 下边界
//    );
//    pRenderTarget->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_ALIASED);
//    //这个需要和滑动条联动
//    float visibileHeigh = rcClient.bottom - m_titleBarHeight - m_controlPanelHeigh;
//    int startIndex = (int)(m_scrollPos / m_trackHeight);
//    int endIndex = startIndex + (int)(visibileHeigh / m_trackHeight) + 2;
//    endIndex = endIndex > tracks.size() ? tracks.size() : endIndex;
//    for (int i=startIndex;i<endIndex;++i)
//    {
//        RECT oneTrackRC = {
//            0,m_titleBarHeight+i*m_trackHeight-m_scrollPos,rcClient.right-m_scrollWidth,m_titleBarHeight+ (i+1) * m_trackHeight - m_scrollPos
//        };
//        DrawOneTrack(pRenderTarget, tracks[i], oneTrackRC);
//    }
//    //结束裁剪
//    pRenderTarget->PopAxisAlignedClip(); // 结束裁剪
//}
//
//void AppUI::DrawOneTrack(ID2D1RenderTarget* pRenderTarget, SongInfo& songInfo, const RECT& rect) {
//    D2D1_COLOR_F trackBgColor = songInfo.state==State::HOVERED ? colorTrackHover : (songInfo.state == State::SELECTED ? colorTrackSelected : colorTrackBg);
//    m_trackBgBrush->SetColor(trackBgColor);
//    D2D1_RECT_F trackRect = D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom);
//    D2D1_RECT_F trackTextRect = D2D1::RectF(rect.left +10, rect.top , rect.right - 10, rect.bottom );
//    pRenderTarget->FillRectangle(&trackRect, m_trackBgBrush);
//
//    D2D1_COLOR_F trackTextColor =  songInfo.state >= State::HOVERED ? colorTrackTextSelected : colorTrackText;;
//    m_trackTextBrush->SetColor(trackTextColor);
//
//    pRenderTarget->DrawText(
//        songInfo.name.c_str(),
//        static_cast<UINT32>(songInfo.name.length()),
//        m_trackTextFormat,
//        trackTextRect,
//        m_trackTextBrush
//    );
//
//    D2D1_COLOR_F trackSecondaryTextColor = songInfo.state >= State::HOVERED ? colorTrackSecondaryTextSelected : colorTrackSecondaryText;
//    m_trackTextBrush->SetColor(trackSecondaryTextColor);
//
//    pRenderTarget->DrawText(
//        songInfo.artist.c_str(),
//        static_cast<UINT32>(songInfo.artist.length()),
//        m_trackSecondaryTextFormat,
//        trackTextRect,
//        m_trackTextBrush
//    );
//    //绘制分割线
//    pRenderTarget->DrawLine(
//        D2D1::Point2F(rect.left, rect.bottom-1),
//        D2D1::Point2F(rect.right, rect.bottom - 1),
//        m_trackSeparetorBrush, 1
//    );
//}
//
//void AppUI::DrawScrollBar(ID2D1RenderTarget* pRenderTarget) {
//    m_trackBgBrush->SetColor(colorScrollBg);
//    D2D1_RECT_F rect = D2D1::RectF(rcScrollTrack.left, rcScrollTrack.top, rcScrollTrack.right, rcScrollTrack.bottom );
//    pRenderTarget->FillRectangle(&rect, m_trackBgBrush);
//    if (m_scrollMax>0)
//    {
//        D2D1_COLOR_F thumbDrawColor = m_scrollThumbState == State::HOVERED ? colorScrollThumbHover : colorScrollThumb;
//        D2D1_ROUNDED_RECT thumbRoundedRect = D2D1::RoundedRect(
//            D2D1::RectF(
//                rcScrollThrumb.left, rcScrollThrumb.top, rcScrollThrumb.right, rcScrollThrumb.bottom
//            ),
//            static_cast<float>(m_scrollWidth/2),
//            static_cast<float>(m_scrollWidth / 2)
//        );
//        m_trackBgBrush->SetColor(thumbDrawColor);
//        pRenderTarget->FillRoundedRectangle(&thumbRoundedRect, m_trackBgBrush);
//    }
//}
//
//void AppUI::DrawControlPanel(ID2D1RenderTarget* pRenderTarget)
//{
//    rcControlPanel = {0,rcClient.bottom-m_controlPanelHeigh,rcClient.right,rcClient.bottom};
//    D2D1_RECT_F controlDrawRect = D2D1::RectF(rcControlPanel.left, rcControlPanel.top, rcControlPanel.right, rcControlPanel.bottom);
//    m_trackBgBrush->SetColor(colorControlPanelBg);
//    DrawBottomRoundedRect(pRenderTarget, controlDrawRect, m_cornerRadius, m_trackBgBrush);
//
//    m_trackBgBrush->SetColor(colorTitleBarBottomLine);
//    pRenderTarget->DrawLine(
//        D2D1::Point2F(0, rcControlPanel.top),
//        D2D1::Point2F(rcClient.right, rcControlPanel.top),
//        m_trackBgBrush, 1.0f
//    );
//
//    //绘制各个按钮
//    int totalWidth = m_BtnSize * 5 + m_BtnSpacing * 4;
//    int startX = (rcControlPanel.right - totalWidth) / 2;
//    int startCenterX = startX + m_BtnSize / 2;
//    int centerY = rcControlPanel.top + m_controlPanelHeigh / 3;
//
//    ButtonType displayModeBtnType = ButtonType::CubeMode;
//    if (m_displayMode == SpectumMode::Wave) {
//        displayModeBtnType = ButtonType::WaveMode;
//    }else if (m_displayMode==SpectumMode::Throw)
//    {
//        displayModeBtnType = ButtonType::ThrowMode;
//    }
//
//    //创建各个btn
//    if (controlBtns.size()==0)
//    {
//        controlBtns.push_back(ControlButton(displayModeBtnType, startCenterX, centerY, m_BtnSize));
//        controlBtns.push_back(ControlButton(ButtonType::Previous, startCenterX +m_BtnSize+m_BtnSpacing, centerY, m_BtnSize));
//        controlBtns.push_back(ControlButton(ButtonType::Play, startCenterX + (m_BtnSize + m_BtnSpacing)*2, centerY, m_BtnSize));
//        controlBtns.push_back(ControlButton(ButtonType::Next, startCenterX + (m_BtnSize + m_BtnSpacing) * 3, centerY, m_BtnSize));
//        controlBtns.push_back(ControlButton(ButtonType::AddFolder, startCenterX + (m_BtnSize + m_BtnSpacing) * 4, centerY, m_BtnSize));
//    }
//    //这里再修正下按钮切换
//    controlBtns[0].type = displayModeBtnType;//播放模式按钮切换
//    if (controlBtns[2].state == State::PRESSED) {
//        if (controlBtns[2].type==ButtonType::Play)
//        {
//            controlBtns[2].type = ButtonType::Pause;
//        }
//        else {
//            controlBtns[2].type = ButtonType::Play;
//        }
//        controlBtns[2].state = State::NONE;
//    }
//
//    for (auto& oneControlBtn:controlBtns)
//    {
//        DrawControlButton(pRenderTarget, oneControlBtn);
//    }
//
//    if (m_playingIndex>=0&&m_playingIndex<=tracks.size()&&tracks.size()>-1)
//    {
//        m_trackTextBrush->SetColor(colorTrackSecondaryText);
//        pRenderTarget->DrawText(
//            tracks[m_playingIndex].name.c_str(),
//            static_cast<UINT32>(tracks[m_playingIndex].name.length()),
//            m_trackTextFormat,
//            D2D1::RectF(rcControlPanel.left+2, rcControlPanel.top+2, rcControlPanel.right, rcControlPanel.top+16),
//            m_trackTextBrush
//        );
//    }
//}
//
//D2D1::ColorF AppUI::colorConvert(int r, int g, int b, int a) {
//    return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
//}
