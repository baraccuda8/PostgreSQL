#pragma once

extern HWND Edit;
extern HWND Execute;
extern HWND ExecuteOut;

extern HINSTANCE hInstance;

std::string utf8_to_cp1251(std::string str);
std::string cp1251_to_utf8(std::string str);

BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
int WinErrorExit(HWND hWnd, const char* lpszFunction);

