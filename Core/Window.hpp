#pragma once

#include <string>
#include <windows.h>
#include <functional>

class Window
{
public:
	Window(int w, int h, HINSTANCE hInstance, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)> userFunction = {});

	void peekMessages();

	int getWidth() const { return m_w; }
	int getHeight() const { return m_h; }
	HWND getHWND() const { return m_hwnd; }

	bool shouldClose() const { return m_quit; }

	Window(const Window& rhs) = delete;
	Window(Window&& rhs) = delete;
	Window& operator=(const Window& rhs) = delete;
	Window& operator=(Window&& rhs) = delete;

private:
	int m_w;
	int m_h;
	HWND m_hwnd;
	std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)> m_userFunction;
	volatile bool m_quit;

	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};