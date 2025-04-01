#pragma once
#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <string>
#include <CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")

class SongListUI
{
public:
	SongListUI(HWND hwndParent);
	~SongListUI();
	HWND getHWND();
	void show();
	void Hide();

	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(HWND hwnd);
	LRESULT OnLButtonDown(HWND hwnd, LPARAM lParam);
	LRESULT OnLButtonUp(HWND hwnd);
	LRESULT OnMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDblClk(HWND hwnd, LPARAM lParam);

	static LRESULT CALLBACK ListProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


private:
	HWND _hwndTable;
	HWND _hwndParent;
	POINT _dragOffset;
	bool _isDragging;
	RECT _tableRect;
	std::vector<std::string> _items;
	int _hoveredRow;
	HBRUSH _normalBrush;
	HBRUSH _hoverBrush;
	HFONT _hFont;

};

