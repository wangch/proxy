// proxy.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "proxy.h"

#include <winsock2.h>
#include <WS2tcpip.h>
#include <Shellapi.h>
#include <process.h>

//#include <thread>
//#include <mutex>
#include <string>
#include <time.h>
#include <vector>
#include <fstream>
#include <map>


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
INT_PTR CALLBACK	set(HWND, UINT, WPARAM, LPARAM);

int start();
int stop();

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

   // Initialize global strings
   LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
   LoadString(hInstance, IDC_PROXY, szWindowClass, MAX_LOADSTRING);
   MyRegisterClass(hInstance);

   WSADATA w;
   if (WSAStartup(MAKEWORD(2, 2), &w) != 0) {
      return -1;
   }

   ULONG_PTR gdiplusToken = 0;
   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

   // Perform application initialization:
   if (!InitInstance (hInstance, nCmdShow))
   {
      return FALSE;
   }

   hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROXY));

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

   WSACleanup();
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
   wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROXY));
   wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground	= (HBRUSH)(CTLCOLOR_DLG);
   wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PROXY);
   wcex.lpszClassName	= szWindowClass;
   wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

   return RegisterClassEx(&wcex);
}

#define CLT_CNT 3

struct R {
   SOCKET s;
   bool b;
};

struct clt_info {
   std::string ip;
   std::map<SOCKET, R> ss;
   //std::mutex m;
   time_t st;
   time_t et;
   bool connected;
};

struct svr_info {
   HWND stk_0;
   HWND stk_1;

   HWND btn_start;
   HWND btn_stop;
   HWND btn_set;

   HWND edt[CLT_CNT];
   HWND btn_cancel[CLT_CNT];

   SOCKET s;
   std::vector<clt_info*> clts;
   int port;

   bool autoproxy;
   bool stop;

   HBRUSH  hbr;

   NOTIFYICONDATA pnid;
} g_svr;

static int init(HWND hWnd) {
   LOGFONT logfont;
   ::memset(&logfont, 0, sizeof(logfont));
   logfont.lfHeight = -14;
   strcpy_s(logfont.lfFaceName, "宋体");
   HFONT hf = ::CreateFontIndirect(&logfont);

   int w0 = 30;
   int h0 = 20;
   int btn_w = 75;
   int btn_h = 23;
   int edt_w = 400;

   Gdiplus::Color txt_color(51, 153, 255);
   Gdiplus::Color bk_color(105, 105, 105);

   HWND edt_0 = CreateWindowEx(0, WC_EDIT, TEXT(""), 
      WS_CHILD |WS_VISIBLE | WS_DISABLED,
      w0, h0*6, edt_w, btn_h, hWnd, (HMENU)IDC_EDT0, hInst, NULL);  

   ::SendMessage(edt_0, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND edt_1 = CreateWindowEx(0, WC_EDIT, TEXT(""), 
      WS_CHILD |WS_VISIBLE | WS_DISABLED,
      w0, h0*8, edt_w, btn_h, hWnd, (HMENU)IDC_EDT1, hInst, NULL);  
   ::SendMessage(edt_1, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND edt_2 = CreateWindowEx(0, WC_EDIT, TEXT(""), 
      WS_CHILD |WS_VISIBLE | WS_DISABLED,
      w0, h0*10, edt_w, btn_h, hWnd, (HMENU)IDC_EDT2, hInst, NULL);  
   ::SendMessage(edt_2, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   
   HWND stk_0 = CreateWindowEx(0, WC_STATIC, TEXT(""), 
      WS_CHILD |WS_VISIBLE,
      w0, h0+4, btn_w+10, btn_h, hWnd, (HMENU)IDC_STK0, hInst, NULL);  
   ::SetWindowText(stk_0, TEXT("我要做代理："));
   ::SendMessage(stk_0, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND stk_1 = CreateWindowEx(0, WC_STATIC, TEXT(""), 
      WS_CHILD |WS_VISIBLE,
      w0, h0*3+10, btn_w*2, btn_h, hWnd, (HMENU)IDC_STK1, hInst, NULL); 
   ::SetWindowText(stk_1, TEXT("连接数量："));
   ::SendMessage(stk_1, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_start = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0+btn_w+40, h0, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_START, hInst, NULL);  
   ::SetWindowText(btn_start, TEXT("开启"));
   ::SendMessage(btn_start, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_stop = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0+btn_w*2+100, h0, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_STOP, hInst, NULL);  
   ::SetWindowText(btn_stop, TEXT("停止"));
   ::SendMessage(btn_stop, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_set = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0+edt_w+20, h0, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_SET, hInst, NULL);  
   ::SetWindowText(btn_set, TEXT("设置"));
   ::SendMessage(btn_set, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));


   HWND btn_cancel0 = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0+edt_w+20, h0*6, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_CANCEL0, hInst, NULL);  
   ::SetWindowText(btn_cancel0, TEXT("断开"));
   ::SendMessage(btn_cancel0, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_cancel1 = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0+edt_w+20, h0*8, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_CANCEL1, hInst, NULL);  
   ::SetWindowText(btn_cancel1, TEXT("断开"));
   ::SendMessage(btn_cancel1, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

   HWND btn_cancel2 = CreateWindowEx(0, WC_BUTTON, TEXT(""), 
      WS_CHILD |WS_VISIBLE|BS_CENTER | BS_FLAT,
      w0+edt_w+20, h0*10, btn_w, btn_h, hWnd, (HMENU)IDC_BTN_CANCEL2, hInst, NULL);  
   ::SetWindowText(btn_cancel2, TEXT("断开"));
   ::SendMessage(btn_cancel2, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));
   
   g_svr.btn_cancel[0] = btn_cancel0;
   g_svr.btn_cancel[1] = btn_cancel1;
   g_svr.btn_cancel[2] = btn_cancel2;
   g_svr.btn_set = btn_set;
   g_svr.btn_start = btn_start;
   g_svr.btn_stop = btn_stop;
   g_svr.edt[0] = edt_0;
   g_svr.edt[1] = edt_1;
   g_svr.edt[2] = edt_2;
   g_svr.stk_0 = stk_0;
   g_svr.stk_1 = stk_1;

   for (int i = 0; i < CLT_CNT; ++i) {
      clt_info* clt = new clt_info;
      clt->connected = false;
      g_svr.clts.push_back(clt);
   }

   g_svr.stop = true;
   g_svr.port = 8080;
   g_svr.autoproxy = false;
   std::ifstream in(TEXT("cfg.ini"));
   if (in) {
      std::string line;
      if (std::getline(in, line)) { // port
         if (line.find("port=") != std::string::npos) {
            std::string p(line.substr(5));
            g_svr.port = ::atoi(p.c_str());
         }
      }
      if (std::getline(in, line)) { // autoproxy
         if (line.find('1') != std::string::npos) {
            g_svr.autoproxy = true;
            start();
            ::EnableWindow(g_svr.btn_start, FALSE);
         }
      }
   }

   // 
   g_svr.pnid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
   g_svr.pnid.hWnd = hWnd;
   g_svr.pnid.uID = IDI_PROXY;
   g_svr.pnid.uFlags= NIF_ICON | NIF_MESSAGE | NIF_TIP;
   g_svr.pnid.uCallbackMessage = WM_MYNOTIFY;
   g_svr.pnid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PROXY));
   ::strcpy(g_svr.pnid.szTip, "proxy,单击隐藏，双击打开");
   Shell_NotifyIcon(NIM_ADD, &g_svr.pnid);

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
      300, 200, 560, 300, NULL, NULL, hInstance, NULL);

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
       case IDC_BTN_SET:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  DialogBox(hInst, MAKEINTRESOURCE(IDD_SETBOX), hWnd, set);
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_START:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  start();
                  ::EnableWindow(g_svr.btn_start, 0);
                  ::EnableWindow(g_svr.btn_stop, 1);
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_STOP:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  stop();
                  ::EnableWindow(g_svr.btn_start, 1);
                  ::EnableWindow(g_svr.btn_stop, 0);
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_CANCEL0:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  g_svr.clts[0]->connected = false;
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_CANCEL1:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  g_svr.clts[1]->connected = false;
               }
               break;
            default:
               break;
            }
         }
         break;
      case IDC_BTN_CANCEL2:
         {
            switch (wmEvent)
            {
            case BN_CLICKED:
               {
                  g_svr.clts[2]->connected = false;
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
         if (g_svr.stk_0 == (HWND)lParam || g_svr.stk_1 == (HWND)lParam ) {
            HDC hdcStatic = (HDC) wParam;
            SetTextColor(hdcStatic, RGB(51, 153, 255));
            SetBkColor(hdcStatic, RGB(105, 105, 105)); 

            if (g_svr.hbr == NULL) {
               g_svr.hbr =CreateSolidBrush(RGB(105, 105, 105));
            }
            return (INT_PTR)g_svr.hbr;
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
      Shell_NotifyIcon(NIM_DELETE, &g_svr.pnid);
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

static int autorun(HWND hDlg) {
   HKEY hKey; 
   //找到系统的启动项 
   LPCTSTR lpRun = "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; 
   char* name = "proxy";
   //打开启动项Key 
   long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
   if(lRet == ERROR_SUCCESS) 
   { 
      HWND hw = ::GetDlgItem(hDlg, IDC_CHK_AUTORUN);
      int r = (int)::SendMessage(hw, BM_GETCHECK, 0, 0);
      if (r == BST_CHECKED) {
         //HWND hw1 = ::GetDlgItem(hDlg, IDC_CHK_AUTOPROXY);
         //r = (int)::SendMessage(hw1, BM_GETCHECK, 0, 0);
         //if (r == BST_CHECKED) {
         //   return 0;
         //}
         ::SendMessage(hw, BM_SETCHECK, BST_UNCHECKED, 0);
         RegDeleteValue(hKey, name);
      } else {
         ::SendMessage(hw, BM_SETCHECK, BST_CHECKED, 0);

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
   return 0;
}

// Message handler for set box.
INT_PTR CALLBACK set(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   switch (message)
   {
   case WM_INITDIALOG:
      {
         LPCTSTR lpRun = "Software\\Microsoft\\Windows\\CurrentVersion\\Run"; 
         char* name = "proxy";
         HKEY hKey; 
         long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_ALL_ACCESS, &hKey); 
         if(lRet == ERROR_SUCCESS) {
            lRet = ::RegQueryValueEx(hKey, name, NULL, NULL, NULL, NULL);
            if (lRet == ERROR_SUCCESS) {
               HWND hw1 = ::GetDlgItem(hDlg, IDC_CHK_AUTORUN);
               ::SendMessage(hw1, BM_SETCHECK, BST_CHECKED, 0);
            }
         }
         RegCloseKey(hKey); 
         if (g_svr.autoproxy) {
            HWND hw1 = ::GetDlgItem(hDlg, IDC_CHK_AUTOPROXY);
            ::SendMessage(hw1, BM_SETCHECK, BST_CHECKED, 0);
         }
      }
      return (INT_PTR)TRUE;

   case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
      {
         EndDialog(hDlg, LOWORD(wParam));
         return (INT_PTR)TRUE;
      }

      if (LOWORD(wParam) == IDC_CHK_AUTORUN) {
         autorun(hDlg);
         return (INT_PTR)TRUE;
      }

      if (LOWORD(wParam) == IDC_CHK_AUTOPROXY) {
         //HWND hw1 = ::GetDlgItem(hDlg, IDC_CHK_AUTORUN);
         //::SendMessage(hw1, BM_SETCHECK, BST_UNCHECKED, 0);
         //autorun(hDlg);
         HWND hw = ::GetDlgItem(hDlg, IDC_CHK_AUTOPROXY);
         int r = (int)::SendMessage(hw, BM_GETCHECK, 0, 0);
         if (r == BST_CHECKED) {
            ::SendMessage(hw, BM_SETCHECK, BST_UNCHECKED, 0);
            g_svr.autoproxy = false;
         } else {
            ::SendMessage(hw, BM_SETCHECK, BST_CHECKED, 0);
            g_svr.autoproxy = true;
         }
         FILE* f = fopen("cfg.ini", "w");
         ::fclose(f);
         std::ofstream of("cfg.ini");
         of << "port=" << g_svr.port << std::endl;
         of << "autoproxy=" <<  g_svr.autoproxy << std::endl;
         return (INT_PTR)TRUE;
      }

      break;
   }
   return (INT_PTR)FALSE;
}

SOCKET connect(std::string server, int port, bool istcp = true) {
   struct addrinfo hints, *res = NULL, *ptr = NULL;
   memset(&hints, 0, sizeof(hints));

   hints.ai_family = AF_INET;
   hints.ai_socktype = istcp ? SOCK_STREAM : SOCK_DGRAM;
   hints.ai_protocol = 0; // istcp ? IPPROTO_TCP : IPPROTO_UDP;
   hints.ai_flags = 0;

   char svc[10];
   sprintf(svc, "%d", port);

   int rc = getaddrinfo(server.c_str(), svc, &hints, &res);
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

static int read(SOCKET s, char* buf, int len) {
   int rd = 0;
   for (;rd != len;) {
      int n = ::recv(s, buf+rd, len-rd, 0);
      if (n <= 0) {
         return -1;
      }
      rd += n;
   }
   return rd;
}

static int write(SOCKET s, char* buf, int len) {
   int wt = 0;
   for (;wt != len;) {
      int n = ::send(s, buf+wt, len-wt, 0);
      if (n == -1) {
         return -1;
      }
      wt += n;
   }
   return wt;
}

struct rr {
   char* buf;
   int len;
};

static int read_res(SOCKET so, SOCKET cs) {
   char buf[1024*8];
   int len = 1024*8;
   int n = ::recv(so, buf, len, 0);
   if (n <= 0) {
      return -1;
   }
   n = write(cs, buf, n);
   if (n == -1) {
      return -1;
   }

   return 0;
}

char* g_cmd[] = {
   "GET",
   "POST",
   "HEAD",
   "PUT",
   "DELETE",
   "TRACE",
   "CONNECT",
   "OPTIONS"
};

bool is_reqh(char* buf) {
   for (int i = 0; i < 8; ++i) {
      if (::strncmp(buf, g_cmd[i], strlen(g_cmd[i])) == 0) {
         return true;
      }
   }
   return false;
}

static SOCKET resolve(char* buf, bool* ssl) {
   char* s = ::strstr(buf, "\r\n");
   char cmd[16] = {0}, uri[4096] = {0}, protocol[16] = {0}, host[NI_MAXHOST] = {0};
   if (::sscanf(buf, "%s %s %s%[^\r\n]\r\n*s", cmd, uri, protocol) < 3) {
      return -1;
   }
      
   int port = 80;
   if (uri[0] != '/') {
      if (::strcmp(cmd, "CONNECT") == 0) {
         return -2;
         //*ssl = true;
         //::sscanf(uri, "%[^:]:%d", host, &port);
      } else {
         ::sscanf(uri+7, "%[^/]/%*s", host);
         char* ss = ::strstr(uri+7, "/");
         ::sprintf(buf, "%s %s %s%s", cmd, ss, protocol, s);
      }
   } else {
      char* s = ::strstr(buf, "Host:");
      s += 6;
      char* s1 = ::strstr(s, "\r\n");
      ::strncpy(host, s, s1 - s);
      s = ::strstr(host, ":");
      if (s) {
         ::sscanf(host, "%[^:]:%d", host, port);
      }
   }
   return connect(host, port);
 }

static int read_req(clt_info* clt, SOCKET cs) {
   char* buf = new char[1024*8];
   int len = 1024*8;
   ::memset(buf, 0, len);

   int rd = 0;
   while (true) {
      int n = ::recv(cs, buf+rd, len-rd, 0);
      if (n <= 0) {
         delete [] buf;
         return -1;
      }
      rd += n;
      int tl = rd;
      R r = clt->ss[cs];
      if (!r.b && is_reqh(buf)) {
         char* s = ::strstr(buf, "\r\n\r\n");
         if (!s) {
            continue;
         }
         if (r.s) {
            ::closesocket(r.s);
         }
         r.s = resolve(buf, &r.b);
         if (r.s == -1) {
            delete [] buf;
            return -1;
         }
         if (r.s == -2) {
            delete [] buf;
            clt->connected = false;
            return -1;
         }

         clt->ss[cs] = r;
         if (r.b) {
            char* https = "HTTP/1.1 200 Connection established\r\n\r\n";
            if (::write(cs, https, strlen(https)) == -1) {
               return -1;
            }
            break;
         }
      }
      if (r.s != 0) {
         n = write(r.s, buf, tl);
         delete [] buf;
         if (n < 0) {
            return -1;
         }
         break;
      }
   }
   return 0; 
}

static int do_res(SOCKET so, SOCKET cs) {
   int n = read_res(so, cs);
   if (n == -1) {
      ::closesocket(cs);
      ::closesocket(so);
      return -1;
   }
   return 0;
}

static int do_req(clt_info* clt, SOCKET cs) {
   int n = read_req(clt, cs);
   if (n == -1) {
      ::closesocket(cs);
      return -1;
   }
   return 0;
}

/*
static int clt_serve(clt_info* clt) {
   while (clt->connected) {
      std::lock_guard<std::mutex> lock(clt->m);
      std::map<SOCKET, SOCKET>::iterator it = clt->ss.begin();
      for (; it != clt->ss.end();) {
         int r = read_req(clt, it->first);
         if (r == -1) {
            it = clt->ss.erase(it);
         } else {
            ++it;
         }
      }
      if (clt->ss.empty()) {
         clt->connected = false;
      }
   }
   return 0;
}
*/

static int accept() {
   typedef std::vector<clt_info*>::iterator IT;

   struct sockaddr_storage addr;
   socklen_t addrlen = sizeof(addr);
   SOCKET cs = ::accept(g_svr.s, (struct sockaddr*)&addr, &addrlen);
   if(cs != -1) {
      socklen_t addr_len = sizeof(struct sockaddr_storage);
      char host[NI_MAXHOST], serv[NI_MAXSERV];

      int r = getnameinfo((struct sockaddr*)&addr, addr_len, 
         host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICSERV);
      bool b = false;
      IT it = g_svr.clts.begin();
      for(; it != g_svr.clts.end(); ++it) {
         clt_info* c = *it;
         if (c->connected && c->ip == host) {
            //std::lock_guard<std::mutex> lock(c->m);
            R r;
            r.b = false;
            r.s = 0;
            c->ss[cs] = r; 
            b = true;
            break;
         }
      }
      if (!b) {
         b = false;
         it = g_svr.clts.begin();
         for(; it != g_svr.clts.end(); ++it) {
            clt_info* c = *it;
            if (!c->connected) {
               //std::lock_guard<std::mutex> lock(c->m);
               c->ip = host;
               R r;
               r.b = false;
               r.s = 0;
               c->ss[cs] = r;
               c->st = time(0);
               c->connected = true;
               b = true;
               break;
            }
         }
      }
      if (!b) {
         ::closesocket(cs); 
      }
   }

   return 0;
}

static unsigned int __stdcall run(void*) {
   fd_set fdread;
   struct timeval timeout;
   typedef std::vector<clt_info*>::iterator IT;

   while(!g_svr.stop) {
      FD_ZERO(&fdread);

      FD_SET(g_svr.s, &fdread);
      int nfds = 5;
      IT it = g_svr.clts.begin(); 
      for(; it != g_svr.clts.end(); ++it) {
         clt_info* c = *it;
         std::map<SOCKET, R>::iterator mt = c->ss.begin();
         for (; mt != c->ss.end(); ++mt) {
            FD_SET(mt->first, &fdread);
            if (mt->second.s != 0) {
               FD_SET(mt->second.s, &fdread);
            }
         }
      }
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;

      nfds += 1; // 

      int rc = ::select(nfds, &fdread, NULL, NULL, &timeout);
      if(rc == -1) {
         break;
      } else if(rc == 0) { // timeout
         continue;
      }

      it = g_svr.clts.begin(); 
      for(; it != g_svr.clts.end(); ++it) {
         clt_info* c = *it;
         if (c->connected) {
            std::map<SOCKET, R>::iterator mt = c->ss.begin();
            for (; mt != c->ss.end();) {
               if (FD_ISSET(mt->first, &fdread)) {
                  if (do_req(c, mt->first) == -1) {
                     ::closesocket(mt->first);
                     ::closesocket(mt->second.s);
                     c->ss.erase(mt++);
                     continue;
                  }
               } 
               ++mt;
            }
            mt = c->ss.begin();
            for (; mt != c->ss.end();) {
               if (FD_ISSET(mt->second.s, &fdread)) {
                  if (do_res(mt->second.s, mt->first) == -1) {
                     ::closesocket(mt->first);
                     ::closesocket(mt->second.s);
                     c->ss.erase(mt++);
                     continue;
                  }
               }
               ++mt;
            }
         }
      }
      if(FD_ISSET(g_svr.s, &fdread)) {
         accept();
      }
   } //while

   ::closesocket(g_svr.s);
   IT it = g_svr.clts.begin(); 
   for(; it != g_svr.clts.end(); ++it) {
      clt_info* c = *it;
      c->connected = false;
   }


   return 0;
}


static SOCKET listen(int port, bool istcp = true) {
   struct addrinfo hints, *res = NULL, *ptr = NULL;
   memset(&hints, 0, sizeof(hints));

   hints.ai_family = AF_INET;
   hints.ai_socktype = istcp ? SOCK_STREAM : SOCK_DGRAM;
   hints.ai_protocol = 0; 
   hints.ai_flags = AI_PASSIVE;
   hints.ai_canonname = NULL;
   hints.ai_addr = NULL;
   hints.ai_next = NULL;

   char svc[10];
   sprintf(svc, "%d", port);

   int rc = getaddrinfo(NULL, svc, &hints, &res);
   if(rc != 0)
      return -1;

   SOCKET fd; 
   for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
      fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      if(fd == -1) {
         continue;
      }

      bool re = true;
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,(const char*)&re, sizeof(bool));
      if(bind(fd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
         if(istcp) {
            if(::listen(fd, 49) != 0) { // tcp listen
               ::closesocket(fd);
               continue;
            }
         }
         break; // OK
      } else {
         closesocket(fd);
      }
   }
   if (ptr == NULL) {
      fd = -1;
   }
   freeaddrinfo(res);
   return fd;
}


static unsigned int __stdcall update(void*) {
   while (!g_svr.stop) {
      typedef std::vector<clt_info*>::iterator IT;
      IT it = g_svr.clts.begin();
      int n = 0;
      for(int i = 0; it != g_svr.clts.end(); ++it, ++i) {
         clt_info* c = *it;
         if (c->connected) {
            time_t t = time(0) - c->st;
            int s = t % 60;
            int m = (t / 60) % 60;
            int h = t / 3600;
            char buf[64];
            ::sprintf(buf, "客户端：%s  连接时长: %d:%d:%d", c->ip.c_str(), h, m, s);
            ::SetWindowText(g_svr.edt[i], buf);
            ::EnableWindow(g_svr.btn_cancel[i], 1);
            ++n;
         } else {
            ::SetWindowText(g_svr.edt[i], "");
            ::EnableWindow(g_svr.btn_cancel[i], 0);
         }
      }
      char buf[32];
      ::sprintf(buf, "连接数量：%d", n);
      ::SetWindowText(g_svr.stk_1, buf);
      ::Sleep(1000);
   }

   for(int i = 0; i < g_svr.clts.size(); ++i) {
      ::SetWindowText(g_svr.edt[i], "");
      ::EnableWindow(g_svr.btn_cancel[i], 0);
   }
   return 0;
}

int start() {
   g_svr.stop = false;

   SOCKET s = listen(g_svr.port);
   if (s == -1) {
      ::MessageBox(0, "系统网络错误", "错误", 0);
      return -1;
   }
   g_svr.s = s;
   //std::thread ts(run);
   //ts.detach();
   unsigned int n;
   ::_beginthreadex(0, 0, run, 0, 0, &n);

   //std::thread tu(update);
   //tu.detach();
   ::_beginthreadex(0, 0, update, 0, 0, &n);
   return 0;
}

int stop() {
   g_svr.stop = true;
   return 0;
}


