/******************************************************************************
Project: TinyMSLicenseViewer
Description: TinyMSLicenseViewer main implementation.
File Name: TinyMSLicenseViewer.cpp
License: The MIT License
******************************************************************************/

#include "stdafx.h"

#include "Version.h"
#include "resource.h"

#include "M2BaseHelpers.h"
#include "M2Win32Helpers.h"

#include <string>

HRESULT GetLicenseStatus(std::wstring& Result)
{
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
	if (FAILED(hr)) return hr;

	M2::CComObject<IWbemLocator*> WbemLocator;
	M2::CComObject<IWbemServices*> WbemServices;

	// ����һ��CLSID_WbemLocator����		
	hr = CoCreateInstance(
		CLSID_WbemLocator,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&WbemLocator));
	if (FAILED(hr)) return hr;

	// ʹ��pLoc���ӵ�"root\cimv2" ��ȡIWbemServices�ӿ�
	hr = WbemLocator->ConnectServer(
		L"ROOT\\CIMV2",
		nullptr,
		nullptr,
		nullptr,
		0,
		nullptr,
		nullptr,
		&WbemServices);
	if (FAILED(hr)) return hr;

	// �������ӵİ�ȫ����
	hr = CoSetProxyBlanket(
		WbemServices,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		nullptr,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr,
		EOAC_NONE);
	if (FAILED(hr)) return hr;

	for (size_t i = 0; i < sizeof(QuerySource) / sizeof(*QuerySource); ++i)
	{
		std::wstring QueryString;

		QueryString += L"SELECT * FROM ";
		QueryString += std::wstring(QuerySource[i]);
		QueryString += L" WHERE PartialProductKey <> null";

		M2::CComObject<IEnumWbemClassObject*> SLPEnumerator;

		hr = WbemServices->ExecQuery(
			L"WQL",
			const_cast<BSTR>(QueryString.c_str()),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			nullptr,
			&SLPEnumerator);
		if (FAILED(hr)) continue;

		for (;;)
		{
			M2::CComObject<IWbemClassObject*> SLPClassObject;
			ULONG uReturn = 0;
			VARIANT vtDescription, vtLicenseStatus;

			SLPEnumerator->Next(
				WBEM_INFINITE,
				1,
				&SLPClassObject,
				&uReturn);

			if (0 == uReturn) break;

			SLPClassObject->Get(
				L"Name",
				0,
				&vtDescription,
				nullptr,
				nullptr);
			SLPClassObject->Get(
				L"LicenseStatus",
				0,
				&vtLicenseStatus,
				nullptr,
				nullptr);


			Result += vtDescription.bstrVal;
			Result += L"\r\n";
			Result += LicenseStatusText[vtLicenseStatus.lVal];
			Result += L"\r\n";
			Result += L"\r\n";

			VariantClear(&vtDescription);
			VariantClear(&vtLicenseStatus);
		}
	}

	return hr;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	std::wstring Result;

	Result += L"TinyMSLicenseViewer " PROJECT_VERSION_STRING "\r\n";
	Result += L"\xA9 M2-Team. All rights reserved.\r\n\r\n";
	Result += L"========�����Ǳ�����΢���Ʒ����Ȩ���========\r\n";
	Result += L"\r\n";

	// ��ʼ�� COM
	HRESULT hr = CoInitializeEx(
		nullptr,
		COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		std::wstring Buffer;
		hr = GetLicenseStatus(Buffer);
		if (SUCCEEDED(hr))
		{
			Result += Buffer;
		}
		else
		{
			Result += L"��ѯʧ��";
		}

		CoUninitialize();
	}

	M2MessageDialog(
		hInstance, 
		nullptr, 
		MAKEINTRESOURCE(IDI_ICON1),
		L"TinyMSLicenseViewer",
		Result.c_str());

	return 0;
}
