// test.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "test.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>

#define MAX_LOADSTRING 100
#define XXX TEXT("BITMAP_XXX")
#define XXX_MUTEX_NAME TEXT("BITMAP_XXX_MUTEX")
#define MEMORY_SIZE 1024 * 1024 * 6

PROCESS_INFORMATION pi;
STARTUPINFO         si;
HANDLE hShare = NULL;
HANDLE hMutex = NULL;

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
LPBYTE bitmap24, bitmap8;
int width = 320;
int height = 480;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

	bitmap24 = (LPBYTE)malloc(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 3);
	memset(bitmap24, 0x00, _msize(bitmap24));

	bitmap8 = (LPBYTE)malloc(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256 + width * height);
	memset(bitmap8, 0x00, _msize(bitmap8));

    // グローバル文字列を初期化しています。
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーションの初期化を実行します:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEST));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


int getdata() {

	DWORD result;
	hShare = CreateFileMapping(
		INVALID_HANDLE_VALUE,                        // ファイルハンドル
		NULL,									     // セキュリティ
		PAGE_READWRITE,							     // 保護
		0,										     // サイズ(上位DWORD)
		MEMORY_SIZE,                                 // size
		XXX);                                        // name

	if (hShare == NULL) {

		// fiald.
		return -1;
	}

	GetStartupInfo(&si);

	// SI設定
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;	// プロセス起動時コマンドプロンプト非表示
	si.wShowWindow = SW_HIDE;			// プロセス起動時コマンドプロンプト非表示	

	// PI設定
	ZeroMemory(&pi, sizeof(pi));

	int iRet = CreateProcess(NULL,					// 実行可能モジュール名
		_tcsdup(TEXT("runner.exe 1 2")),			// コマンドライン文字列
		NULL,					                    // セキュリティー記述子
		NULL,					                    // セキュリティー記述子
		FALSE,				                        // ハンドル継承オプション
		CREATE_NEW_CONSOLE,	                        // 作成フラグ
		NULL,					                    // 新しい環境のブロック
		NULL,					                    // カレントディレクトリ名
		&si,					                    // スタートアップ情報
		&pi);				                        // プロセス情報

	if (iRet == 0) {

		// fiald.
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		CloseHandle(hShare);
		hShare = NULL;

		return 1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	GetExitCodeProcess(pi.hProcess, &result);

	WCHAR szStr[256];
	wsprintf(szStr, _T("result.%d"), result);
	MessageBox(NULL, szStr, _T("ok"), MB_OK);


	if (result != 0) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hShare);
		hShare = NULL;

		return result;
	}


	hMutex = CreateMutex(NULL, FALSE, XXX_MUTEX_NAME);
	if (hMutex == NULL) {

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hShare);
		hShare = NULL;

		return -1;
	}

	iRet = WaitForSingleObject(hMutex, INFINITE);
	if (iRet == WAIT_FAILED) {
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hShare);
		hShare = NULL;
	}
	
	LPBYTE MEMORY_DATA = (LPBYTE)MapViewOfFile(hShare,       // ハンドル
		FILE_MAP_READ,					// アクセスモード(読取)
		0,								// サイズ(上位DWORD)
		0,								// サイズ(下位DWORD)
		MEMORY_SIZE);                   // 共有メモリサイズ

	if (MEMORY_DATA == NULL) {
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hShare);
		hShare = NULL;
		return -1;
	}

	memset(bitmap24, 0x00, _msize(bitmap24));
	memset(bitmap8, 0x00, _msize(bitmap8));

	memcpy(bitmap24, MEMORY_DATA,  _msize(bitmap24));
	memcpy(bitmap8,  MEMORY_DATA + _msize(bitmap24), _msize(bitmap8));

	if (MEMORY_DATA != NULL)
	{
		UnmapViewOfFile(MEMORY_DATA);
	}

	// 
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	// 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	// 
	CloseHandle(hShare);
	hShare = NULL;

	return 0;
}

//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   CreateWindowW(
	   TEXT("BUTTON"), TEXT("Save."),
	   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	   330, 10, 100, 30,
	   hWnd, (HMENU)666, hInstance, NULL
   );

   CreateWindowW(
	   TEXT("BUTTON"), TEXT("Run."),
	   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	   220, 10, 100, 30,
	   hWnd, (HMENU)999, hInstance, NULL
   );


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウの描画
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static WCHAR szStr[256];
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case 666:
				FILE *bitmap8file;
				FILE *bitmap24file;

				fopen_s(&bitmap8file, "bitmap8.bmp", "wb");
				fopen_s(&bitmap24file, "bitmap24.bmp", "wb");

				fwrite(bitmap8,  1, _msize(bitmap8),  bitmap8file);
				fwrite(bitmap24, 1, _msize(bitmap24), bitmap24file);

				fclose(bitmap8file);
				fclose(bitmap24file);

				MessageBoxA(nullptr, "files be saved.", "ok", MB_OK);
				break;
			case 999:
				
				getdata();

				InvalidateRect(hWnd, NULL, TRUE);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
			SetTextColor(hdc, 0xFF << 16);
			wsprintf(szStr, _T("Draw bitmap."), 10);

			TextOut(hdc, 10, 10, szStr, lstrlen(szStr));

			GetCurrentDirectory(MAX_PATH, szStr);

			TextOut(hdc, 500, 10, szStr, lstrlen(szStr));

			StretchDIBits(
				hdc,
				10,
				60,
				width , height,
				0,
				0,
				width , height,
				bitmap8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256,
				(LPBITMAPINFO)(bitmap8 + sizeof(BITMAPFILEHEADER)),
				DIB_RGB_COLORS,
				SRCCOPY
			);

			StretchDIBits(
				hdc,
				400,
				60,
				width, height,
				0,
				0,
				width, height,
				bitmap24 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER),
				(LPBITMAPINFO)(bitmap24 + sizeof(BITMAPFILEHEADER)),
				DIB_RGB_COLORS,
				SRCCOPY);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		free(bitmap24);
		free(bitmap8);
        PostQuitMessage(0);
        break;
	case WM_CREATE:

		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
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
