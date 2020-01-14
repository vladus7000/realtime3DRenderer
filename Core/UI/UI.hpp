#pragma once

#include <windows.h>

class Resources;
void uiInit(Resources* res);
void uiDeinit();
int uiInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void uiDraw();