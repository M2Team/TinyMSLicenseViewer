// TinyMSLicenseViewer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "Version.h"
#include "resource.h"
#include "M2Win32Helpers.h"

#include <string>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	const wchar_t* LicenseStatusText[] =
	{
		L"δ����Ȩ",
		L"����Ȩ",
		L"��ʼ������",
		L"���ӿ�����(KMS ���֤���ڻ�Ӳ��������������)",
		L"�����������",
		L"֪ͨ",
		L"�ӳ��Ŀ�����"
	};

	const wchar_t* QuerySource[] =
	{
		L"SoftwareLicensingProduct",
		L"PartialProductKey"
	};
	
	HRESULT hr = S_OK;

	std::wstring Result;

	Result += L"TinyMSLicenseViewer " PROJECT_VERSION_STRING "\r\n";
	Result += L"\xA9 M2-Team. All rights reserved.\r\n\r\n";
	Result += L"========�����Ǳ�����΢���Ʒ����Ȩ���========\r\n";
	Result += L"\r\n";

	IWbemLocator *pLoc = nullptr;
	IWbemServices *pSvc = nullptr;

	do
	{
		// ��ʼ�� COM
		hr = CoInitializeEx(
			nullptr,
			COINIT_MULTITHREADED);
		if (FAILED(hr)) break;

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
		if (FAILED(hr)) break;

		// ����һ��CLSID_WbemLocator����		
		hr = CoCreateInstance(
			CLSID_WbemLocator,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator,
			(LPVOID *)&pLoc);
		if (FAILED(hr)) break;

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
		if (FAILED(hr)) break;

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
		if (FAILED(hr)) break;

		for (size_t i = 0; i < sizeof(QuerySource) / sizeof(*QuerySource); ++i)
		{
			std::wstring QueryString;

			QueryString += L"SELECT * FROM ";
			QueryString += std::wstring(QuerySource[i]);
			QueryString += L" WHERE PartialProductKey <> null";

			IEnumWbemClassObject* pSLPEnumerator = nullptr;

			hr = pSvc->ExecQuery(
				L"WQL",
				const_cast<BSTR>(QueryString.c_str()),
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


					Result += std::wstring(vtDescription.bstrVal) + L"\r\n";
					Result += std::wstring(LicenseStatusText[vtLicenseStatus.lVal]) + L"\r\n";
					Result += L"\r\n";

					VariantClear(&vtDescription);
					VariantClear(&vtLicenseStatus);

					pSLPClassObject->Release();
				}
				pSLPEnumerator->Release();
			}
		}

		M2MessageDialog(hInstance, nullptr, MAKEINTRESOURCE(IDI_ICON1), L"TinyMSLicenseViewer", Result.c_str());

	} while (false);

	if (pSvc) pSvc->Release();
	if (pLoc) pLoc->Release();
	CoUninitialize();
	return 0;
}
