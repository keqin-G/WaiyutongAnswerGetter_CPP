#include <Windows.h>
#include <iostream>
#include <fstream>

#include "Waiyutong.h"

#define IDB_GET      111
#define IDE_USERNAME 112
#define IDE_PASSWORD 113
#define IDE_HID      114
#define IDE_ANSWER   115
#define IDB_ABOUT    116
#define IDM_MENU     117

//global variable
HWND g_hWnd = NULL;
std::string username, password;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void GetAnswer();
void ReadConfig();
void SaveConfig();


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//调出控制台
	AllocConsole();
	freopen("conout$", "w", stdout);

	TCHAR szAppName[] = TEXT("Waiyutong");
	HWND hWnd = NULL;
	MSG msg;
	WNDCLASS wndClass;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = "IDI_MENU";
	wndClass.lpszClassName = szAppName;

	if (!RegisterClass(&wndClass))
	{
		MessageBox(NULL, TEXT("Failed in function RegisterClass()"), TEXT("Error"), MB_OK);
		return 0;
	}

	g_hWnd = hWnd = CreateWindow(szAppName, TEXT("Waiyutong Answer Getter"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 850, hWnd, NULL, hInstance, NULL);
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND usernameStaticHWND = NULL;
	HWND passwordStaticHWND = NULL;
	HWND hidStaticHWND = NULL;
	HWND usernameEditHWND = NULL;
	HWND passwordEditHWND = NULL;
	HWND hidEditHWND = NULL;
	HWND answerEditHWND = NULL;
	HWND getButtonHWND = NULL;
	

	TEXTMETRIC textMetric;
	HDC hdc = GetDC(hWnd);
	GetTextMetrics(hdc, &textMetric);
	static HFONT hFont = CreateFont(20, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("微软雅黑"));//创建字体;

	switch (message)
	{
	case WM_CREATE:
		HMENU hMenu;
		hMenu = CreateMenu();
		AppendMenu(hMenu, MF_STRING | MF_POPUP, IDB_ABOUT, TEXT("关于"));//将主菜单追加到窗体
		SetMenu(hWnd, hMenu);

		usernameStaticHWND = CreateWindow(TEXT("static"), TEXT("账号"), WS_CHILD | WS_VISIBLE | SS_LEFT,							10,                                       10,														  textMetric.tmAveCharWidth * 10, textMetric.tmHeight + 8, hWnd, NULL,			        LPCREATESTRUCT(lParam)->hInstance, NULL);
		passwordStaticHWND = CreateWindow(TEXT("static"), TEXT("密码"), WS_CHILD | WS_VISIBLE | SS_LEFT,							10,									      10 + textMetric.tmHeight + 10,							  textMetric.tmAveCharWidth * 10, textMetric.tmHeight + 8, hWnd, NULL,			        LPCREATESTRUCT(lParam)->hInstance, NULL);
		hidStaticHWND      = CreateWindow(TEXT("static"), TEXT("Hid"), WS_CHILD | WS_VISIBLE | SS_LEFT,									10,                                       10 + textMetric.tmHeight*3 + 4,                             textMetric.tmAveCharWidth * 10, textMetric.tmHeight + 8, hWnd, NULL,                  LPCREATESTRUCT(lParam)->hInstance, NULL);
		usernameEditHWND   = CreateWindow(TEXT("edit"),   NULL,             WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,				10 + textMetric.tmAveCharWidth * 10 + 10, 10,														  textMetric.tmMaxCharWidth * 25, textMetric.tmHeight + 8, hWnd, (HMENU)(IDE_USERNAME), LPCREATESTRUCT(lParam)->hInstance, NULL);
		passwordEditHWND   = CreateWindow(TEXT("edit"),   NULL,             WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,				10 + textMetric.tmAveCharWidth * 10 + 10, 10 + textMetric.tmHeight + 10,							  textMetric.tmMaxCharWidth * 25, textMetric.tmHeight + 8, hWnd, (HMENU)(IDE_PASSWORD), LPCREATESTRUCT(lParam)->hInstance, NULL);
		hidEditHWND        = CreateWindow(TEXT("edit"),   NULL,             WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,				10 + textMetric.tmAveCharWidth * 10 + 10, 10 + textMetric.tmHeight*3 + 4,                             textMetric.tmMaxCharWidth * 25, textMetric.tmHeight + 8, hWnd, (HMENU)(IDE_HID),	    LPCREATESTRUCT(lParam)->hInstance, NULL);
		answerEditHWND     = CreateWindow(TEXT("edit"),   NULL,             WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER | ES_MULTILINE, 10,                                       10 + textMetric.tmHeight*3 + 10 + textMetric.tmHeight + 10, 565,                            700,                 hWnd, (HMENU)(IDE_ANSWER),   LPCREATESTRUCT(lParam)->hInstance, NULL);
		getButtonHWND      = CreateWindow(TEXT("button"), TEXT("Get"),      WS_CHILD | WS_VISIBLE | SS_CENTER,                          500,                                      10,                                                         40,                             40,                  hWnd, (HMENU)(IDB_GET),      LPCREATESTRUCT(lParam)->hInstance, NULL);
		
		SendMessage(answerEditHWND, WM_SETFONT, (WPARAM)(hFont), TRUE);
		SendMessage(usernameEditHWND, WM_SETFONT, (WPARAM)(hFont), TRUE);
		SendMessage(passwordEditHWND, WM_SETFONT, (WPARAM)(hFont), TRUE);
		SendMessage(hidEditHWND, WM_SETFONT, (WPARAM)(hFont), TRUE);
		SendMessage(usernameStaticHWND, WM_SETFONT, (WPARAM)(hFont), TRUE);
		SendMessage(passwordStaticHWND, WM_SETFONT, (WPARAM)(hFont), TRUE);
		SendMessage(hidStaticHWND, WM_SETFONT, (WPARAM)(hFont), TRUE);

		SetWindowText(answerEditHWND, TEXT("Hid留空自动获取最新作业.\r\n输入日期(如:2020-02-20)获取当日作业.这会遍历所有的作业,如非必要尽量别用.\r\n输入Hid获取该Hid对应的作业.\r\n"));	
		ReadConfig();
		if (!username.empty())
		{
			SetWindowText(usernameEditHWND, username.c_str());
		}
		if (!password.empty())
		{
			SetWindowText(passwordEditHWND, password.c_str());
		}
	
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDB_ABOUT:
			//use messageboxw to show ©
			MessageBoxW(hWnd, TEXT(L"Copyright © 2020 by Godzhang.All rights reserved."), TEXT(L"关于"), MB_OK);
			break;
		case IDB_GET:
			GetAnswer();
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}


void GetAnswer()
{
	char u[32], p[32], h[32]; 
	SendMessage(GetDlgItem(g_hWnd, IDE_USERNAME), WM_GETTEXT, 32, (LPARAM)(u));
	SendMessage(GetDlgItem(g_hWnd, IDE_PASSWORD), WM_GETTEXT, 32, (LPARAM)(p));
	SendMessage(GetDlgItem(g_hWnd, IDE_HID), WM_GETTEXT, 32, (LPARAM)(h));
	std::string username(u);
	std::string password(p);
	std::string hid(h);


	Waiyutong *w = new Waiyutong;
	if (!w->Login(username.c_str(), password.c_str()))
	{
		SetWindowText(GetDlgItem(g_hWnd, IDE_ANSWER), TEXT("登陆失败!请检查你的账户和网络设置.\r\n"));
		return;
	}
	if (hid.empty())
	{
		if (!w->GetLatestHomework())
		{
			SetWindowText(GetDlgItem(g_hWnd, IDE_ANSWER), TEXT("获取作业失败!请联系作者.\r\n"));
			return;
		}
	}
	else if (hid.find('-') != -1)
	{
		if (!w->GetHomeworkByDate(hid))
		{
			SetWindowText(GetDlgItem(g_hWnd, IDE_ANSWER), TEXT("获取作业失败!请检查日期.\r\n"));
			return;
		}
	}

	std::string answer;
	auto a = w->GetAnswer();
	for (std::vector<std::string> b : a)
	{
		for (int i = 0; i < b.size(); i++)
		{
			std::string c = b.at(i);
			answer += c;
		}
	}

	SetWindowText(GetDlgItem(g_hWnd, IDE_ANSWER), answer.c_str());
	SaveConfig();

}

void ReadConfig()
{
	char p[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, p);
	std::string path(p);
	path += "wag";
	
	std::ifstream in(path.c_str(), std::ios::in);
	if (!in)
	{
		return;
	}

	std::getline(in, username, '\n');
	std::getline(in, password, '\n');
	in.close();

// 	SetWindowText(GetDlgItem(g_hWnd, IDE_USERNAME), username.c_str());
// 	SetWindowText(GetDlgItem(g_hWnd, IDE_PASSWORD), password.c_str());
}


void SaveConfig()
{
	char p[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, p);
	std::string path(p);
	path += "wag";

	std::ofstream out(path.c_str(), std::ios::out);
	if (!out)
	{
		return;
	}
	
	char username[32], password[32];
	SendMessage(GetDlgItem(g_hWnd, IDE_USERNAME), WM_GETTEXT, 32, (LPARAM)(username));
	SendMessage(GetDlgItem(g_hWnd, IDE_PASSWORD), WM_GETTEXT, 32, (LPARAM)(password));

	out << username << std::endl << password;
	out.close();

}