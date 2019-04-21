#include "Window.h"

namespace
{
	typedef std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> LocalWindowProc;

	std::map<HWND, LocalWindowProc> LocalWindowProcMap;

	LRESULT CALLBACK GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		auto item = LocalWindowProcMap.find(hWnd);
		return (item != LocalWindowProcMap.end()) ?
			item->second(hWnd, msg, wParam, lParam) : DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

Window::~Window()
{
	Close();
}

HRESULT Window::Setup(HINSTANCE hInstance, LPCTSTR title)
{
	if (handle_ != nullptr)
	{
		Close();
	}

	HRESULT result;

	WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW,
		GlobalWindowProc,
		0, 0,
		hInstance,
		LoadIcon(hInstance, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		(HBRUSH)COLOR_WINDOW,
		nullptr,
		title,
		LoadIcon(hInstance, IDI_APPLICATION)
	};

	result = RegisterClassEx(&wcex);
	if (FAILED(result))
	{
		return result;
	}

	handle_ = CreateWindow(
		title,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
	if (handle_ == nullptr)
	{
		return S_FALSE;
	}

	LocalWindowProcMap[handle_] = [this](auto hWnd, auto msg, auto wParam, auto lParam)
	{
		return WindowProc_(hWnd, msg, wParam, lParam);
	};

	instanceHandle_ = hInstance;
	title_ = title;

	return S_OK;
}

HRESULT Window::Move(int x, int y)
{
	if (!SetWindowPos(handle_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER))
	{
		return S_FALSE;
	}
	return S_OK;
}

HRESULT Window::Resize(int width, int height)
{
	RECT window;
	if (!GetWindowRect(handle_, &window))
	{
		return S_FALSE;
	}

	RECT client;
	if (!GetClientRect(handle_, &client))
	{
		return S_FALSE;
	}

	auto marginW = (window.right - window.left) - (client.right - client.left);
	auto marginH = (window.bottom - window.top) - (client.bottom - client.top);

	if (!MoveWindow(handle_, window.left, window.top, width + marginW, height + marginH, TRUE))
	{
		return S_FALSE;
	}

	return S_OK;
}


HRESULT Window::Open(int nCmdShow)
{
	if (!ShowWindow(handle_, nCmdShow))
	{
		return S_FALSE;
	}
	return S_OK;
}


HRESULT Window::Close()
{
	if (handle_ == nullptr)
	{
		return S_OK;
	}

	if (!UnregisterClass(title_, instanceHandle_))
	{
		return S_FALSE;
	}

	if (!CloseWindow(handle_))
	{
		return S_FALSE;
	}

	LocalWindowProcMap.erase(handle_);

	handle_ = nullptr;
	instanceHandle_ = nullptr;
	title_ = nullptr;

	return S_OK;
}

void Window::SetEventHandler(WindowEvent ev, std::function<void(WindowEventArg*)> handler)
{
	eventHandlers_[ev] = handler;
}

void Window::MessageLoop(std::function<void()> onIdle)
{
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		auto nextMsg = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (nextMsg)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			onIdle();
		}
	}
}

LRESULT Window::WindowProc_(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}

		case WM_KEYUP:
		{
			if (wParam == VK_ESCAPE)
			{
				PostMessage(hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}

		case WM_SIZE:
		{
			ResizeEventArg e = {
				static_cast<int>(lParam & 0xFFFF),
				static_cast<int>((lParam >> 16) & 0xFFFF)
			};
			InvokeEventHandler_(WindowEvent::Resize, &e);
			break;
		}

		default:
			break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Window::InvokeEventHandler_(WindowEvent ev, WindowEventArg* e)
{
	auto item = eventHandlers_.find(ev);
	if (item != eventHandlers_.end())
	{
		item->second(e);
	}
}
