#pragma once
#include <Windows.h>
#include "WindowEvent.h"
#include <map>
#include <functional>

class Window
{
public:
	~Window();

	HRESULT Setup(HINSTANCE hInstance, LPCTSTR title);
	HRESULT Move(int x, int y);
	HRESULT Resize(int width, int height);
	HRESULT Open(int nCmdShow = SW_SHOWNORMAL);
	HRESULT Close();

	void SetEventHandler(WindowEvent ev, std::function<void(WindowEventArg*)> handler);
	void MessageLoop(std::function<void()> onIdle);

public:
	const HWND Handle() { return handle_; }

private:
	HWND handle_ = nullptr;
	HINSTANCE instanceHandle_ = nullptr;
	LPCTSTR title_ = nullptr;

	std::map<WindowEvent, std::function<void(WindowEventArg*)>> eventHandlers_;

	LRESULT WindowProc_(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void InvokeEventHandler_(WindowEvent ev, WindowEventArg* e);
};

