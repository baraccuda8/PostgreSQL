
#include <Windows.h>
#include <thread>
#include <iostream>
#include <strsafe.h>
#include <commctrl.h>
#include <sstream> 
#include <iostream>
#include <iomanip>


#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "resource.h"

#include "Source.h"
#include "pgconnection.h"

#define SQLFileName "PostgreSQL.dat"

#define testselect "--DELETE FROM sheet;\r\n" \
"SELECT id, zone, melt, partno, pack, sheet, pos, temper, news FROM sheet ORDER BY id;" 
//SELECT id AS "ИД", zone AS "Зона", melt AS "Плавка", partno AS "Партия", pack AS "Пачка", sheet AS "Лист", pos  AS "Позицыя" FROM sheet WHERE news = 0 ORDER BY id;
//используете параметр ASC 
//для сортировки строк в порядке возрастания и 
//параметр DESC для сортировки строк в порядке убывания.
//Если вы опустите параметр ASC или DESC, в ORDER BY по умолчанию будет использоваться ASC.

#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifdef _WIN64
#define DLLRESULT LRESULT
#else
#define DLLRESULT INT_PTR
#endif


#define FUNCTION_LINE_NAME std::string (std::string(" File: ") + std::string( __FILE__ ) + std::string("; Function: ") + std::string( __FUNCTION__ ) + std::string ("; Line ") + std::to_string(__LINE__) + std::string ("; "))
#define __FUN(__s) std::string( \
			  std::string(__s) + std::string( "(File: ") + std::string ( __FILE__ ) + std::string( ";Function: ") + std::string ( __FUNCTION__ ) + std::string ( ";Line: ") + std::to_string(__LINE__) + std::string (";)")).c_str()
#define FLN			std::string (std::string( "(File: ") + std::string ( __FILE__ ) + std::string( ";Function: ") + std::string ( __FUNCTION__ ) + std::string ( ";Line: ") + std::to_string(__LINE__) + std::string (";)"))
#define FLN2		std::string (std::string("\nFile: ") + std::string ( __FILE__ ) + std::string("\nFunction: ") + std::string ( __FUNCTION__ ) + std::string ("\nLine: ") + std::to_string(__LINE__) + std::string ("\n"))

HWND Edit = NULL;
HWND Execute = NULL;
HWND Save = NULL;
HWND ExecuteOut = NULL;

HINSTANCE hInstance = NULL;
HACCEL hAccelTable = NULL;

//auto pgbackend = std::make_shared<PGBackend>();

//Структура инициализации добавочных контролов
//
INITCOMMONCONTROLSEX initcontrol ={sizeof(INITCOMMONCONTROLSEX),
      ICC_LISTVIEW_CLASSES    //listview, header
    | ICC_TREEVIEW_CLASSES    // treeview, tooltips
    | ICC_BAR_CLASSES         // toolbar, statusbar, trackbar, tooltips
    | ICC_TAB_CLASSES         // tab, tooltips
    | ICC_UPDOWN_CLASS        // updown
    | ICC_PROGRESS_CLASS      // progress
    | ICC_ANIMATE_CLASS       // animate
    | ICC_WIN95_CLASSES
    | ICC_DATE_CLASSES        // month picker, date picker, time picker, updown
    | ICC_USEREX_CLASSES      // comboex
    | ICC_COOL_CLASSES        // rebar (coolbar) control
    | ICC_INTERNET_CLASSES
    | ICC_PAGESCROLLER_CLASS  // page scroller
    | ICC_NATIVEFNTCTL_CLASS  // native font control
#if (NTDDI_VERSION >= NTDDI_WINXP)
    | ICC_STANDARD_CLASSES
    | ICC_LINK_CLASS
#endif

};



extern std::string m_dbhost;
extern std::string m_dbport;
extern std::string m_dbname;
extern std::string m_dbuser;
extern std::string m_dbpass;

extern PGConnection conn;



//Класс главного окна
#define szGlobalWindowClass "PostgreSQLclient"
std::string szTitle = "Postgre SQL client";

//Главное окно
HWND GlobalWindow = NULL;


#pragma region Функции гравного окна

//Закрытие программы
LRESULT Quit()
{
    PostQuitMessage(0);
    return 0;
}

#pragma endregion
int winsize = 300;


LRESULT Size(LPARAM lParam)
{
    int cx = LOWORD(lParam);
    int cy = HIWORD(lParam);

    winsize = (cy - 25) / 2;

    MoveWindow(Edit, 0, 0, cx, winsize, true);
    MoveWindow(Execute, cx - 300, winsize, 150, 25, true);
    MoveWindow(Save, cx - 150, winsize, 150, 25, true);
    MoveWindow(ExecuteOut, 0, winsize+25, cx, cy - winsize - 25, true);
    
    return 0;
}


LRESULT onCommand1(WPARAM wParam)
{
    if(wParam != BN_CLICKED) return 0;
    std::string ss;
    int len = GetWindowTextLength(Edit) + 1;
    ss.resize(len);
    GetWindowText(Edit, &ss[0], len);
    testConnection(ss);
    return 0;
}
extern std::string StrOut;
LRESULT onCommand2(WPARAM wParam)
{
    if(wParam != BN_CLICKED) return 0;
    if(StrOut.length())
    {
        std::fstream s1("out.csv", std::fstream::binary | std::fstream::out);
        s1 << StrOut;
        s1.close();

    }
    return 0;
}

LRESULT Command(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int command = LOWORD(wParam);
    if(command == 11) return onCommand1(HIWORD(wParam));
    if(command == 12) return onCommand2(HIWORD(wParam));
    return DefWindowProc(hWnd, message, wParam, lParam);
}


//Функция обработки сообщений главного окна
LRESULT CALLBACK GlobalWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //if(message == WM_USER)    return CommandUser(hWnd, message, wParam, lParam);
    //if(message == WM_KEYDOWN)
    //{
        //if(wParam == VK_RETURN) return Quit();
        //if(wParam == VK_ESCAPE) return Quit();
    //}

    if(message == WM_COMMAND)return Command(hWnd, message, wParam, lParam);
    if(message == WM_SIZE) return Size(lParam);
    if(message == WM_DESTROY) return Quit();

    return DefWindowProc(hWnd, message, wParam, lParam);
}

//Инициализация класса главного окна
void MyRegisterGlobalClass()
{
    WNDCLASSEX cex0 ={0};
    cex0.cbSize = sizeof(WNDCLASSEX);
    cex0.style          = CS_HREDRAW | CS_VREDRAW;
    cex0.lpfnWndProc    = GlobalWindowProc;
    cex0.cbClsExtra     = 0;
    cex0.cbWndExtra     = 0;
    cex0.hInstance      = hInstance;
    cex0.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    cex0.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    cex0.hbrBackground  = (HBRUSH)(CTLCOLOR_DLG + 1);
    cex0.lpszMenuName   = NULL;// MAKEINTRESOURCE(IDC_DATACOLLACTION);
    cex0.lpszClassName  = szGlobalWindowClass;
    cex0.hIconSm        = LoadIcon(cex0.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    if(cex0.hInstance && !RegisterClassExA(&cex0))
        throw std::exception(__FUN(std::string("Ошибка регистрации класса окна cex0: " + std::string(szGlobalWindowClass))));
}

std::string pKey = "хабраbarracudabarracudaхабра";
char sCommand[0xFFFF];
char sCommand2[0xFFFF];

void encode(char* pText, int len)
{
    for(int i = 0; i < len; i++)
        pText[i] = (byte)(pText[i] ^ pKey[i % pKey.length()]);
}

void SaveConnect()
{
    std::stringstream pass;
    pass << m_dbhost << std::endl
        << m_dbport << std::endl
        << m_dbname << std::endl
        << m_dbuser << std::endl
        << m_dbpass;

    std::string p = pass.str();
    memset(sCommand2, 0, 0xFFFF);
    strcpy_s(sCommand2, 255, p.c_str());;
    encode(sCommand2, p.length());

    std::ofstream s(SQLFileName, std::ios::binary | std::ios::out | std::ios::trunc);
    if(s.is_open())
    {
        s.write(sCommand2, p.length());
        s.close();
    }
}

bool LoadConnect()
{
    memset(sCommand2, 0, 0xFFFF);
    std::ifstream s(SQLFileName, std::ios::binary | std::ios::in);
    if(s.is_open())
    {
        s.read(sCommand2, 1024);
        int len = (int)s.gcount();
        s.close();
        encode(sCommand2, len);
        std::vector <std::string>split;
        boost::split(split, sCommand2, boost::is_any_of("\n"));
        if(split.size() == 5)
        {
            m_dbhost = split[0];
            m_dbport = split[1];
            m_dbname = split[2];
            m_dbuser = split[3];
            m_dbpass = split[4];
            return TRUE;
        }
    }
    return FALSE;
}

void LoadCommand()
{
    memset(sCommand, 0, 0xFFFF);
    std::ifstream s("command.txt", std::ios::binary | std::ios::in);
    if(s.is_open())
    {
        s.read(sCommand, 0xFFFF);
        int len = (int)s.gcount();
        s.close();
    }
}

HFONT Font = NULL;
//Создание главного окна


void InitGlobalInstance()
{
    szTitle += " '" + m_dbhost + "'";
    GlobalWindow = CreateWindowExA(0/*WS_EX_TOPMOST*/,
                                   szGlobalWindowClass, szTitle.c_str(),
                                   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                   NULL, NULL, hInstance, NULL);


    if(!GlobalWindow) throw std::exception((FLN2 + std::string("Ошибка создания окна : " + std::string(szGlobalWindowClass))).c_str());
    LoadCommand();
    Edit = CreateWindowExA(0, "EDIT", sCommand, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_MULTILINE, 0, 0, winsize, 500, GlobalWindow, (HMENU)10, hInstance, NULL);
    if(!Edit) throw std::exception((FLN2 + std::string("Ошибка создания окна : EDIT")).c_str());

    

    //EXECUTE
    Execute = CreateWindowExA(0, "BUTTON", "Выполнить", WS_CHILD | WS_VISIBLE | WS_BORDER/* | BS_FLAT*/, 0, 500, winsize, 25, GlobalWindow, (HMENU)11, hInstance, NULL);
    if(!Execute) throw std::exception((FLN2 + std::string("Ошибка создания окна : Execute")).c_str());

    Save = CreateWindowExA(0, "BUTTON", "Сохранить", WS_CHILD | WS_VISIBLE | WS_BORDER/* | BS_FLAT*/, 150, 500, winsize, 25, GlobalWindow, (HMENU)12, hInstance, NULL);
    if(!Execute) throw std::exception((FLN2 + std::string("Ошибка создания окна : Save")).c_str());

    ExecuteOut = CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_MULTILINE , 0, 0, winsize, 500, GlobalWindow, (HMENU)12, hInstance, NULL);
    if(!ExecuteOut) throw std::exception((FLN2 + std::string("Ошибка создания окна : ExecuteOut")).c_str());

    HDC hDc = GetDC(NULL);
    int DeviceCapsCX = GetDeviceCaps(hDc, LOGPIXELSY);
    Font = CreateFont(-MulDiv(12, DeviceCapsCX, 72), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, "Fixedsys");

    SendMessage(ExecuteOut, WM_SETFONT, (WPARAM)Font, 0);

    ShowWindow(GlobalWindow, SW_SHOWDEFAULT);
    UpdateWindow(GlobalWindow);

}

//Инициализация главного окна
bool InitGlobalWindow(HINSTANCE hInst)
{
    MyRegisterGlobalClass();
    InitGlobalInstance();
    return true;
}


DLLRESULT InitDialog(HWND hWnd)
{
    CenterWindow(hWnd, NULL);
    SetWindowText(GetDlgItem(hWnd, IDC_EDIT1), m_dbhost.c_str());
    SetWindowText(GetDlgItem(hWnd, IDC_EDIT2), m_dbport.c_str());
    SetWindowText(GetDlgItem(hWnd, IDC_EDIT3), m_dbname.c_str());
    SetWindowText(GetDlgItem(hWnd, IDC_EDIT4), m_dbuser.c_str());
    SetWindowText(GetDlgItem(hWnd, IDC_EDIT5), m_dbpass.c_str());
    return 0;
}

DLLRESULT CommandDialog(HWND hWnd, WPARAM wParam)
{
    if(wParam == IDOK)
    {
        char ss[256];
        GetWindowText(GetDlgItem(hWnd, IDC_EDIT1), ss, 256);    m_dbhost = ss;
        GetWindowText(GetDlgItem(hWnd, IDC_EDIT2), ss, 256);    m_dbport = ss;
        GetWindowText(GetDlgItem(hWnd, IDC_EDIT3), ss, 256);    m_dbname = ss;
        GetWindowText(GetDlgItem(hWnd, IDC_EDIT4), ss, 256);    m_dbuser = ss;
        GetWindowText(GetDlgItem(hWnd, IDC_EDIT5), ss, 256);    m_dbpass = ss;

        conn.connection();
        if(conn.connections)
        {
            //SaveConnect();
            EndDialog(hWnd, FALSE);
        }
    }
    if(wParam == IDCANCEL)
    {
        EndDialog(hWnd, FALSE);
        Quit();
    }
    return 0;
}

DLLRESULT CALLBACK bagSave(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_INITDIALOG)return InitDialog(hWnd);
    if(message == WM_COMMAND) return CommandDialog(hWnd, wParam);
    return (0);
}

void CurrentDir()
{
    char ss[256] = "";
    GetModuleFileNameA(NULL, ss, 255);
    char ss2[256];
    strcpy_s(ss2, 256, ss);
    char* s1 = ss2;
    char* s2 = NULL;
    while(s1 && *s1)
    {
        if(*s1 == '\\')s2 = s1;
        s1++;
    }
    if(s2)
    {
        *s2 = 0;
        SetCurrentDirectory(ss2);
    }
}


int APIENTRY wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    CurrentDir();
    LoadConnect();
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, bagSave);

    if(conn.connections)
    {
        SaveConnect();

        try
        {
            hInstance = hInst;
            if(!hInstance) throw std::exception((FUNCTION_LINE_NAME + std::string("Ошибка запуска программы : hInst = NULL")).c_str());
            if(!InitCommonControlsEx(&initcontrol)) throw std::exception(__FUN("Ошибка загрузки модуля CommonControlsEx"));

            InitGlobalWindow(hInstance);

            MSG msg;
            hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
            if(!hAccelTable) throw std::exception(__FUN("Ошибка инициализации таблицы Accelerators"));

            // Цикл основного сообщения:
            //while(msg.message != WM_QUIT)
            while(GetMessage(&msg, nullptr, 0, 0) && msg.message != WM_QUIT)
            {
                if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

        }
        catch(std::exception& exc)
        {
            WinErrorExit(NULL, exc.what());
        }
        catch(...)
        {
            WinErrorExit(NULL, "Unknown error.");
        }
    }
}

int main(int argc, char const* argv[])
{
    //system("chcp 1251");
    //std::vector<std::shared_ptr<std::thread>> vec;
    //for(size_t i = 0; i < 50; ++i)
    {
        //vec.push_back(std::make_shared<std::thread>(std::thread
        testConnection("INSERT INTO kpvl (ids, name) VALUES (15, 'Var1');");
        //    );
        //vec.push_back(std::make_shared<std::thread>(std::thread(
        testConnection("SELECT max(ID) FROM kpvl;");
        //));
    }

    //for(auto& i : vec)
    //{
    //    i.get()->join();
    //}


    return 0;
}
