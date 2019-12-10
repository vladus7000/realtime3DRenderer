#include "Window.hpp"

Window* g_window;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		g_window->m_quit = true;
		PostQuitMessage(0);
		return 0;
	}

	return g_window->m_userFunction(hwnd, msg, wParam, lParam);
}

Window::Window(int w, int h, HINSTANCE hInstance, std::function<LRESULT(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)> userFunction)
{
	g_window = this;
	m_w = w;
	m_h = h;
	m_userFunction = userFunction;
	m_quit = false;
	MSG  msg;
	WNDCLASSW wc = { 0 };

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = L"Main Window";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);

	RegisterClassW(&wc);
	m_hwnd = CreateWindowW(wc.lpszClassName, L"Main Window",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, w, h, NULL, NULL, hInstance, NULL);

}

void Window::peekMessages()
{
	MSG msg;
    const int maxMessages(10);
    int processed(0);
	while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
        processed++;
        if (processed >= maxMessages)
        {
            return;
        }
	}
}
