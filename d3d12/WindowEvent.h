#pragma once

enum class WindowEvent
{
	Resize,
};

struct WindowEventArg {};

struct ResizeEventArg : public WindowEventArg
{
	int Width;
	int Height;

	ResizeEventArg(int w, int h)
		: Width(w), Height(h)
	{}
};

