// proxyclt.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "proxyclt.h"

#include <winsock2.h>
#include <WS2tcpip.h>

#include <fstream>
#include <string>
#include <Shellapi.h>
#include <objbase.h>

#define MAX_LOADSTRING 100

#define WM_MYNOTIFY WM_USER+12345

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	check(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

   ULONG_PTR gdiplusToken = 0;
   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PROXYCLT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROXYCLT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
   Gdiplus::GdiplusShutdown(gdiplusToken);

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROXYCLT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(CTLCOLOR_DLG);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PROXYCLT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

struct clt_info {
   HWND combo_box_addr;
   HWND btn_ckeck;
   HWND btn_conn;
   HWND btn_setproxy;
   HWND btn_cancel;
   HWND btn_autorun;
   HWND btn_open_proxy_server;

   HBRUSH  hbr;

   NOTIFYICONDATA pnid;
} g_clt;


static int init(HWND hWnd) {
   LOGFONT logfont;
   ::memset(&logfont, 0, sizeof(logfont));
   logfont.lfHeight = -14;
   strcpy_s(logfont.lfFaceName, "宋体");
   HFONT hf = ::CreateFontIndirect(&logfont);

   HWND combo_box_addr = CreateWindowEx(0, WC_COMBOBOX, TEXT(""), 
      CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
      10, 30, 200, 200, hWnd, (HMENU)IDC_COMBOBOX_ADDR, hInst, NULL);
   ::SendMessage(combo_box_addr, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   std::ifstream in(TEXT("addr.ini"));
   bool b = false;
   if (in) {
      std::string line;
      while (std::getline(in, line)) {
         if (line.length() > 10) {
            ::SendMessage(combo_box_addr, CB_ADDSTRING, WPARAM(0), (LPARAM)line.c_str());
            b = true;
         }
      }
   }
   if (b) {
      ::SendMessage(combo_box_addr, CB_SETCURSEL , WPARAM(0), 0);
   }

   int btn_w = 75;
   int btn_h = 23;

   int w0 = 230;
   int w1 = w0+btn_w+20;
   int h0 = 30;
   int h1 = h0+btn_h + 20;
   int h2 = h1+btn_h + 20;

   HWND btn_ckeck = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0, h0, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_CHECK, hInst, NULL);  
   ::SetWindowText(btn_ckeck, TEXT("查看"));
   ::SendMessage(btn_ckeck, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_conn = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD | WS_VISIBLE|BS_CENTER | BS_FLAT,
      w1, h0, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_CONN, hInst, NULL);
   ::SetWindowText(btn_conn, TEXT("连接"));
   ::SendMessage(btn_conn, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_setproxy = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |  WS_VISIBLE|BS_CENTER | BS_FLAT,
      60, h1, 100, btn_h, hWnd, (HMENU)IDC_BTN_SETPROXY, hInst, NULL);
   ::SetWindowText(btn_setproxy, TEXT("代理上网"));
   ::SendMessage(btn_setproxy, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_cancel = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD | WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0, h1, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_CANCEL, hInst, NULL);
   ::SetWindowText(btn_cancel, TEXT("取消"));
   ::SendMessage(btn_cancel, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_open_proxy_server = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD | WS_VISIBLE|BS_CENTER | BS_FLAT,
      w1, h2, 100, btn_h, hWnd, (HMENU)IDC_BTN_OPENPROXY, hInst, NULL);
   ::SetWindowText(btn_open_proxy_server, TEXT("我要做代理"));
   ::SendMessage(btn_open_proxy_server, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_autorun = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD | BS_CHECKBOX | WS_VISIBLE,
      10, h2, btn_w*2, btn_h, hWnd, (HMENU)IDC_BTN_AUTORUN, hInst, NULL);
   ::SetWindowText(btn_autorun, TEXT("开机自动启动"));
   ::SendMessage(btn_autorun, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   g_clt.btn_autorun = btn_autorun;
   g_clt.btn_cancel = btn_cancel;
   g_clt.btn_ckeck = btn_ckeck;
   g_clt.btn_conn = btn_conn;
   g_clt.btn_open_proxy_server = btn_open_proxy_server;
   g_clt.btn_setproxy = btn_setproxy;
   g_clt.combo_box_addr = combo_box_addr;
   g_clt.hbr = NULL;

   g_clt.pnid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
   g_clt.pnid.hWnd = hWnd;
   g_clt.pnid.uID = IDI_PROXYCLT;
   g_clt.pnid.uFlags= NIF_ICON | NIF_MESSAGE | NIF_TIP;
   g_clt.pnid.uCallbackMessage = WM_MYNOTIFY;
   g_clt.pnid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PROXYCLT));
   ::strcpy(g_clt.pnid.szTip, "proxyclt,单击隐藏，双击打开");
   Shell_NotifyIcon(NIM_ADD, &g_clt.pnid);

   LPCTSTR lpRun = "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; 
   char* name = "proxyclt";
   HKEY hKey; 
   long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_ALL_ACCESS, &hKey); 
   if(lRet == ERROR_SUCCESS) {
      lRet = ::RegQueryValueEx(hKey, name, NULL, NULL, NULL, NULL);
      if (lRet == ERROR_SUCCESS) {
         ::SendMessage(btn_autorun, BM_SETCHECK, BST_CHECKED, 0);
      }
   }
   RegCloseKey(hKey); 

   return 0;
}
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      300, 200, 460, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ::SetMenu(hWnd, NULL);

   init(hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

static void draw(HWND hWnd, HDC dc) {
   RECT rc;
   ::GetClientRect(hWnd, &rc);
   int x = rc.left;
   int y = rc.top;
   int w = rc.right - rc.left;
   int h = rc.bottom - rc.top;

   HBITMAP memmap = ::CreateCompatibleBitmap(dc, w, h);
   HDC mdc = ::CreateCompatibleDC(dc); 
   HBITMAP oldmap = (HBITMAP)::SelectObject(mdc, (HGDIOBJ)memmap);

   Gdiplus::Graphics* g = new Gdiplus::Graphics(mdc);
   Gdiplus::SolidBrush br(Gdiplus::Color(105,105,105));   
   g->FillRectangle(&br, x, y, w, h);
 
   BitBlt(dc, 
      x, y, w, h, 
      mdc, x, y, SRCCOPY);
   delete g;
   ::SelectObject(mdc, oldmap);
   ::DeleteObject(memmap); 
   ::DeleteDC(mdc);
}

SOCKET connect(std::string server, char* port, bool istcp = true) {
   struct addrinfo hints, *res = NULL, *ptr = NULL;
   memset(&hints, 0, sizeof(hints));

   hints.ai_family = AF_INET;
   hints.ai_socktype = istcp ? SOCK_STREAM : SOCK_DGRAM;
   hints.ai_protocol = 0; // istcp ? IPPROTO_TCP : IPPROTO_UDP;
   hints.ai_flags = 0;

   int rc = getaddrinfo(server.c_str(), port, &hints, &res);
   if (rc != 0) {
      return -1;
   }

   SOCKET fd; 
   for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
      fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      if(fd == -1) {
         continue;
      }
      if(::connect(fd, ptr->ai_addr, ptr->ai_addrlen) != -1) {
         break; // ok
      }
      closesocket(fd);
      fd = -1;
   }

   if (ptr == NULL) {
      fd = -1;
   }

   freeaddrinfo(res);
   return fd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
      case IDC_COMBOBOX_ADDR:
         {
         }
         break;
      case IDC_BTN_CHECK:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
         			DialogBox(hInst, MAKEINTRESOURCE(IDD_CHECKBOX), hWnd, check);
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_CONN:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_SETPROXY:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  char buf[64];
                  ::GetWindowText(g_clt.combo_box_addr, buf, 64);
                  SetConnectionOptions(NULL, buf);
                  int n = (int)SendMessage(g_clt.combo_box_addr, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
                  if (n == CB_ERR) {
                     ::SendMessage(g_clt.combo_box_addr, CB_ADDSTRING, 0, (LPARAM)buf);
                  }
                  int cnt = (int)::SendMessage(g_clt.combo_box_addr, CB_GETCOUNT, 0, 0);
                  FILE* f = fopen("addr.ini", "w");
                  fclose(f);
                  std::ofstream of("addr.ini", std::ios_base::out | std::ios_base::trunc);
                  for (int i = 0; i < cnt; ++i) {
                     ::SendMessage(g_clt.combo_box_addr, CB_GETLBTEXT, i, (LPARAM)buf);
                     of << buf << std::endl;
                  }
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_CANCEL:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  DisableConnectionProxy(NULL);
                  char buf[64];
                  ::GetWindowText(g_clt.combo_box_addr, buf, 64);
                  char host[40], port[16];
                  if (::sscanf(buf, "%[^:]:%s", host, port) < 2) {
                     break;
                  }

                  SOCKET s = connect(host, port);
                  if (s == -1) {
                     break;
                  }
                  char* ss = "CONNECT XXX HTTP/1.1\r\n\r\n";
                  ::send(s, ss, ::strlen(ss), 0); 
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_AUTORUN:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  HKEY hKey; 
                  //找到系统的启动项 
                  LPCTSTR lpRun = "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; 
                  char* name = "proxyclt";
                  //打开启动项Key 
                  long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
                  if(lRet == ERROR_SUCCESS) 
                  { 
                     int r = (int)::SendMessage(g_clt.btn_autorun, BM_GETCHECK, 0, 0);
                     if (r == BST_CHECKED) {
                        ::SendMessage(g_clt.btn_autorun, BM_SETCHECK, BST_UNCHECKED, 0);
                        RegDeleteValue(hKey, name);
                     } else {
                        ::SendMessage(g_clt.btn_autorun, BM_SETCHECK, BST_CHECKED, 0);

                        //写入注册表,开机自启动 
                        char pFileName[MAX_PATH] = {0}; 
                        //得到程序自身的全路径 
                        DWORD dwRet = GetModuleFileName(NULL, pFileName, MAX_PATH); 
                        //添加一个子Key,并设置值 // 下面的""是应用程序名字（不加后缀.exe）
                        lRet = RegSetValueEx(hKey, name, 0, REG_SZ, (BYTE *)pFileName, dwRet); 

                        //关闭注册表 
                        RegCloseKey(hKey); 
                        if(lRet != ERROR_SUCCESS) 
                        {  
                           MessageBox(0, "系统参数错误,不能随系统启动", "错误", 0); 
                        } 
                     }
                  }
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_OPENPROXY:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                  HINSTANCE ret = ::ShellExecute(NULL, "open", "proxy.exe", NULL, NULL, SW_SHOWNORMAL);
                  if (ERROR_FILE_NOT_FOUND == (long)ret) {
                     ::MessageBox(0, "没有此程序", "错误", 0);
                  }
               }
               break;
            default:
               break;
            }
         }
         break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
      draw(hWnd, hdc);
		EndPaint(hWnd, &ps);
      break;
   case WM_CTLCOLORSTATIC:
      {
         if (g_clt.btn_autorun == (HWND)lParam) {
            HDC hdcStatic = (HDC) wParam;
            SetTextColor(hdcStatic, RGB(51, 153, 255));
            SetBkColor(hdcStatic, RGB(105, 105, 105)); 

            if (g_clt.hbr == NULL) {
               g_clt.hbr =CreateSolidBrush(RGB(105, 105, 105));
            }
            return (INT_PTR)g_clt.hbr;
         }
      }
      break;
   case WM_MYNOTIFY:
      {
         if (lParam == WM_LBUTTONDOWN) {
            ::ShowWindow(hWnd, SW_HIDE);
         }
         if (lParam == WM_LBUTTONDBLCLK) {
            ::ShowWindow(hWnd, SW_SHOW);
         }
      }
      break;
	case WM_DESTROY:
      Shell_NotifyIcon(NIM_DELETE, &g_clt.pnid);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK check(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
      {
         //int index = (int)::SendMessage(g_clt.combo_box_addr, CB_GETCURSEL, 0, 0);
         char buf[64];
         ::GetWindowText(g_clt.combo_box_addr, buf, 64);
         //int len = (int)::SendMessage(g_clt.combo_box_addr, CB_GETLBTEXT, (WPARAM)index, (LPARAM)buf);
         int l = ::strstr(buf, ":") - buf;
         if (l < 0) {
            MessageBox(0, "地址输入错误", "错误", 0);
            break;
         }

         char ip[40];
         ::strncpy_s(ip, 40, buf, l);
         ip[l] = 0;
         int ret = 0;
         int t = ping(10, ip, &ret);
         if (t < 0) {
            MessageBox(0, "地址输入错误", "错误", 0);
            break;
         }

         ::sprintf_s(buf, "代理服务器： %s", ip);
         SetDlgItemText(hDlg, IDC_STATIC1, buf);

         ::sprintf_s(buf, "请求10次， 返回 %d 次", ret);
         SetDlgItemText(hDlg, IDC_STATIC2, buf);

         ::sprintf_s(buf, "平均耗时 %d 毫秒", t);
         SetDlgItemText(hDlg, IDC_STATIC3, buf);

         int level = 0;
         if (t < 5 && ret == 10) {
            level = 1;
         } else if (t < 10 && ret > 8) {
            level = 2;
         } else if (t < 20 && ret > 6) {
            level = 3;
         } else if (t < 50 && ret > 4) {
            level = 4;
         } else {
            level = 5;
         }

         ::sprintf_s(buf, "级别为 %d 级", level);
         SetDlgItemText(hDlg, IDC_STATIC4, buf);
      }
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
