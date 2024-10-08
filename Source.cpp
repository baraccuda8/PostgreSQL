#include <Windows.h>
#include <thread>
#include <iostream>
#include <strsafe.h>
#include <commctrl.h>


#include "Source.h"

std::string cp1251_to_utf8(std::string str)
{
    //return str;
    std::string res;
    std::wstring ures;

    int result_u, result_c;

    result_u = MultiByteToWideChar(1251, 0, &str[0], -1, 0, 0);
    if(!result_u) return "";

    ures.resize(result_u);
    if(!MultiByteToWideChar(1251, 0, &str[0], -1, &ures[0], result_u)) return "";

    result_c = WideCharToMultiByte(CP_UTF8, 0, &ures[0], -1, 0, 0, 0, 0);
    if(!result_c) return "";

    res.resize(result_c);
    if(!WideCharToMultiByte(CP_UTF8, 0, &ures[0], -1, &res[0], result_c, 0, 0)) return "";

    return res;
}

std::string utf8_to_cp1251(std::string str)
{
    //return str;
    if(!str.length()) return "";

    std::wstring res;

    int convertResult;
    convertResult = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    if(convertResult <= 0)
        return "";

    res.resize(convertResult);
    if(MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &res[0], (int)res.size()) <= 0)
        return "";

    convertResult = WideCharToMultiByte(CP_ACP, 0, res.c_str(), (int)res.length(), NULL, 0, NULL, NULL);
    if(convertResult <= 0)
        return "";

    str.resize(convertResult);
    if(WideCharToMultiByte(CP_ACP, 0, res.c_str(), (int)res.length(), &str[0], (int)str.size(), NULL, NULL) <= 0)
        return "";

    return str;
}

//Вывод строки ошибки выполнения программы
int WinErrorExit(HWND hWnd, const char* lpszFunction)
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf);

    //if(MainLogger) MainLogger->error(std::string((char*)lpDisplayBuf));
    MessageBox(hWnd, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK | MB_SYSTEMMODAL | MB_ICONERROR);
    //SendDebug((LPCTSTR)(lpDisplayBuf));

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    PostQuitMessage(0);
    return 1;
}


BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
{
    RECT rcChild, rcParent;
    int  cxChild, cyChild, cxParent, cyParent, cxScreen, cyScreen, xNew, yNew;
    HDC  hdc;

    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

    if(hwndParent)
    {
        GetWindowRect(hwndParent, &rcParent);
        cxParent = rcParent.right - rcParent.left;
        cyParent = rcParent.bottom - rcParent.top;
    }
    else
    {
        cxParent = GetSystemMetrics(SM_CXMAXIMIZED);
        cyParent = GetSystemMetrics(SM_CYMAXIMIZED);
        rcParent.left = 0;
        rcParent.top = 0;
    }

    hdc = GetDC(hwndChild);
    cxScreen = GetDeviceCaps(hdc, HORZRES);
    cyScreen = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(hwndChild, hdc);

    xNew = rcParent.left + (cxParent - cxChild) / 2;
    if(xNew < 0)xNew = 0; else if((xNew + cxChild) > cxScreen) xNew = cxScreen - cxChild;

    yNew = rcParent.top + (cyParent - cyChild) / 2;
    if(yNew < 0)yNew = 0; else if((yNew + cyChild) > cyScreen) yNew = cyScreen - cyChild;
    return SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}