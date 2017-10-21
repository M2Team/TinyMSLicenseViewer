// TinyMSLicenseViewer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

// Per-Monitor DPI Aware ֧��
#ifndef PER_MONITOR_DPI_AWARE_SUPPORT
#define PER_MONITOR_DPI_AWARE_SUPPORT

#include <ShellScalingApi.h>

inline HRESULT WINAPI GetDpiForMonitor(
	_In_ HMONITOR hmonitor,
	_In_ MONITOR_DPI_TYPE dpiType,
	_Out_ UINT *dpiX,
	_Out_ UINT *dpiY)
{
	HINSTANCE hInstWinSta = LoadLibraryW(L"SHCore.dll");
	if (hInstWinSta == nullptr) return E_NOINTERFACE;

	typedef HRESULT(WINAPI * PFN_GDFM)(
		HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

	PFN_GDFM pGetDpiForMonitor =
		(PFN_GDFM)GetProcAddress(hInstWinSta, "GetDpiForMonitor");
	if (pGetDpiForMonitor == nullptr) return E_NOINTERFACE;

	return pGetDpiForMonitor(hmonitor, dpiType, dpiX, dpiY);
}

// �����Ի���Per-Monitor DPI Aware֧��(����Win10)
inline BOOL EnablePerMonitorDialogScaling()
{
	typedef BOOL(WINAPI *PFN_EnablePerMonitorDialogScaling)();

	PFN_EnablePerMonitorDialogScaling pEnablePerMonitorDialogScaling =
		(PFN_EnablePerMonitorDialogScaling)GetProcAddress(
			GetModuleHandleW(L"user32.dll"), (LPCSTR)2577);

	if (pEnablePerMonitorDialogScaling) return pEnablePerMonitorDialogScaling();
	return -1;
}

// �����Ӵ���DPI��Ϣ(Win10 TH1��TH2)
inline BOOL EnableChildWindowDpiMessage(
	_In_ HWND hWnd,
	_In_ BOOL bEnable)
{
	typedef BOOL(WINAPI *PFN_EnableChildWindowDpiMessage)(HWND, BOOL);

	PFN_EnableChildWindowDpiMessage pEnableChildWindowDpiMessage =
		(PFN_EnableChildWindowDpiMessage)GetProcAddress(
			GetModuleHandleW(L"user32.dll"), "EnableChildWindowDpiMessage");

	if (pEnableChildWindowDpiMessage)
		return pEnableChildWindowDpiMessage(hWnd, bEnable);
	return -1;
}

// �����Ӵ���DPI��Ϣ(Win10 RS1���Ժ�)
inline BOOL NtUserEnableChildWindowDpiMessage(
	_In_ HWND hWnd,
	_In_ BOOL bEnable)
{
	typedef BOOL(WINAPI *PFN_NtUserEnableChildWindowDpiMessage)(HWND, BOOL);

	PFN_NtUserEnableChildWindowDpiMessage pNtUserEnableChildWindowDpiMessage =
		(PFN_NtUserEnableChildWindowDpiMessage)GetProcAddress(
			GetModuleHandleW(L"win32u.dll"), "NtUserEnableChildWindowDpiMessage");

	if (pNtUserEnableChildWindowDpiMessage)
		return pNtUserEnableChildWindowDpiMessage(hWnd, bEnable);
	return -1;
}

#endif // !PER_MONITOR_DPI_AWARE_SUPPORT

// ȫ�ֱ���
int g_xDPI = USER_DEFAULT_SCREEN_DPI;
int g_yDPI = USER_DEFAULT_SCREEN_DPI;

// �Ի�����Ϣ�ص�
LRESULT CALLBACK MsgDlgCB(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
	{
		HWND hEdit = FindWindowExW(hWnd, nullptr, L"Edit", nullptr);
		SetWindowPos(hEdit, nullptr, 0, 0, LOWORD(lParam), HIWORD(lParam), 0);
		break;
	}
	case WM_DPICHANGED:
	{
		g_xDPI = LOWORD(wParam);
		g_yDPI = HIWORD(wParam);

		HWND hEdit = FindWindowExW(hWnd, nullptr, L"Edit", nullptr);

		// �����������
		HFONT hFont = CreateFontW
		(
			-MulDiv(16, g_xDPI, USER_DEFAULT_SCREEN_DPI),
			0,
			0,
			0,
			FW_NORMAL,
			false,
			false,
			false,
			DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS,
			CLIP_CHARACTER_PRECIS,
			CLEARTYPE_NATURAL_QUALITY,
			FF_MODERN,
			L"΢���ź�"
		);

		// ����Edit������
		SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

		RECT* const prcNewWindow = (RECT*)lParam;
		SetWindowPos(
			hWnd,
			nullptr,
			prcNewWindow->left,
			prcNewWindow->top,
			prcNewWindow->right - prcNewWindow->left,
			prcNewWindow->bottom - prcNewWindow->top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}


// �ڶԻ�������ʾ��Ϣ
int WINAPI MessageDialog(
	_In_ HINSTANCE hInstance,
	_In_opt_ HWND hWndParent,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_ int nDlglogWidth,
	_In_ int nDlglogHeight)
{
	EnablePerMonitorDialogScaling();
	
	WNDCLASSEXW wcex;
	RECT Rect;
	MSG msg;
	int cxScreen = GetSystemMetrics(SM_CXSCREEN);
	int cyScreen = GetSystemMetrics(SM_CYSCREEN);

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MsgDlgCB;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"M2::MessageDialog";
	wcex.hIconSm = nullptr;

	// ע�ᴰ����
	RegisterClassExW(&wcex);

	// ��������
	HWND hWnd = CreateWindowExW
	(
		WS_EX_DLGMODALFRAME,
		wcex.lpszClassName,
		lpCaption,
		WS_OVERLAPPEDWINDOW | DS_MODALFRAME,
		0,
		0,
		0,
		0,
		hWndParent,
		nullptr,
		hInstance,
		nullptr
	);
	if (!hWnd) return -1;

	EnableChildWindowDpiMessage(hWnd, TRUE);
	NtUserEnableChildWindowDpiMessage(hWnd, TRUE);

	// ��ȡDPI����

	HRESULT hr = E_FAIL;

	hr = GetDpiForMonitor(
		MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST),
		MDT_EFFECTIVE_DPI, (UINT*)&g_xDPI, (UINT*)&g_yDPI);
	if (hr != S_OK)
	{
		g_xDPI = GetDeviceCaps(GetDC(hWnd), LOGPIXELSX);
		g_yDPI = GetDeviceCaps(GetDC(hWnd), LOGPIXELSY);
	}

	// ��ȡ����λ��
	GetWindowRect(hWnd, &Rect);

	// ���д���
	SetWindowPos(
		hWnd,
		nullptr,
		(cxScreen - MulDiv(nDlglogWidth, g_xDPI, USER_DEFAULT_SCREEN_DPI)) / 2,
		(cyScreen - MulDiv(nDlglogHeight, g_yDPI, USER_DEFAULT_SCREEN_DPI)) / 2,
		MulDiv(nDlglogWidth, g_xDPI, USER_DEFAULT_SCREEN_DPI),
		MulDiv(nDlglogHeight, g_yDPI, USER_DEFAULT_SCREEN_DPI),
		0);

	// ��ȡ���ڿͻ���λ��
	GetClientRect(hWnd, &Rect);

	// ����Edit�ؼ�
	HWND hEdit = CreateWindowExW
	(
		0,
		L"Edit",
		lpText,
		ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_CHILD | WS_VISIBLE,
		0,
		0,
		Rect.right - Rect.left,
		Rect.bottom - Rect.top,
		hWnd,
		nullptr,
		hInstance,
		nullptr
	);
	if (!hEdit) return -1;

	// �����������
	HFONT hFont = CreateFontW
	(
		-MulDiv(16, g_xDPI, USER_DEFAULT_SCREEN_DPI),
		0,
		0,
		0,
		FW_NORMAL,
		false,
		false,
		false,
		DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS,
		CLEARTYPE_NATURAL_QUALITY,
		FF_MODERN,
		L"΢���ź�"
	);

	// ����Edit������
	SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

	// ����Windows������
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);

	// ��ʾ�����´���
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// ����Ϣѭ��
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}

const wchar_t szHeader[] =
L"TinyMSLicenseViewer 1.2\r\n"
L"\xA9 M2-Team. All rights reserved.\r\n\r\n"
L"========�����Ǳ�����΢���Ʒ����Ȩ���========\r\n\r\n";

const wchar_t *strQuery[2] =
{
	L"SELECT * FROM SoftwareLicensingProduct WHERE PartialProductKey <> null",
	L"SELECT * FROM OfficeSoftwareProtectionProduct WHERE PartialProductKey <> null"
};

const wchar_t *LicenseStatusText[7] =
{
	L"δ����Ȩ",
	L"����Ȩ",
	L"��ʼ������",
	L"���ӿ�����(KMS ���֤���ڻ�Ӳ��������������)",
	L"�����������",
	L"֪ͨ",
	L"�ӳ��Ŀ�����"
};


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	
	HRESULT hr;
	wchar_t *szBuffer = nullptr;
	wchar_t *szResult = nullptr;
	IWbemLocator *pLoc = nullptr;
	IWbemServices *pSvc = nullptr;

	// ��ʼ�� COM
	hr = CoInitializeEx(
		nullptr,
		COINIT_MULTITHREADED);
	if (FAILED(hr)) goto Exit;
	
	// ���ý��̰�ȫ����
	hr = CoInitializeSecurity(
		nullptr,
		-1,
		nullptr,
		nullptr,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr,
		EOAC_NONE, 
		nullptr);
	if (FAILED(hr)) goto Exit;

	szBuffer = (wchar_t*)malloc(32768 * sizeof(wchar_t));
	if (!szBuffer) goto Exit;

	szResult = (wchar_t*)malloc(32768 * sizeof(wchar_t));
	if (!szResult) goto Exit;

	wcscpy_s(szResult, 32768, szHeader);
	
	// ����һ��CLSID_WbemLocator����		
	hr = CoCreateInstance(
		CLSID_WbemLocator,
		nullptr, 
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, 
		(LPVOID *)&pLoc);
	if (FAILED(hr)) goto Exit;

	// ʹ��pLoc���ӵ�"root\cimv2" ��ȡIWbemServices�ӿ�
	hr = pLoc->ConnectServer(
		L"ROOT\\CIMV2",
		nullptr, 
		nullptr, 
		nullptr,
		0,
		nullptr,
		nullptr,
		&pSvc);
	if (FAILED(hr)) goto Exit;

	// �������ӵİ�ȫ����
	hr = CoSetProxyBlanket(
		pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		nullptr,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr, 
		EOAC_NONE);
	if (FAILED(hr)) goto Exit;

	for (size_t i = 0; i < 2; i++)
	{
		IEnumWbemClassObject* pSLPEnumerator = nullptr;

		hr = pSvc->ExecQuery(
			L"WQL",
			(BSTR)strQuery[i],
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			nullptr,
			&pSLPEnumerator);

		if (SUCCEEDED(hr))
		{
			IWbemClassObject *pSLPClassObject = nullptr;
			ULONG uReturn = 0;
			VARIANT vtDescription, vtLicenseStatus;

			while (true)
			{
				pSLPEnumerator->Next(
					WBEM_INFINITE,
					1,
					&pSLPClassObject, 
					&uReturn);

				if (0 == uReturn) break;

				pSLPClassObject->Get(
					L"Name",
					0,
					&vtDescription,
					nullptr,
					nullptr);
				pSLPClassObject->Get(
					L"LicenseStatus",
					0,
					&vtLicenseStatus, 
					nullptr, 
					nullptr);

				_swprintf_c(
					szBuffer, 32768,
					L"%s\r\n%s\r\n\r\n",
					vtDescription.bstrVal,
					LicenseStatusText[vtLicenseStatus.lVal]);
				wcscat_s(szResult, 32768, szBuffer);

				pSLPClassObject->Release();
			}
			pSLPEnumerator->Release();
		}
	}

	MessageDialog(hInstance, nullptr, szResult, L"TinyMSLicenseViewer", 500, 300);

Exit:
	if (pSvc) pSvc->Release();
	if (pLoc) pLoc->Release();
	if (szResult) free(szResult);
	if (szBuffer) free(szBuffer);
	CoUninitialize();
	return 0;
}
