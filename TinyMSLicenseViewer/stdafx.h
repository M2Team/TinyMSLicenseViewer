// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

// Ϊ����ͨ�������õľ���
#if _MSC_VER >= 1200
// �������Ż����ܳ��ֵľ��棨ȥ��δ���ú������ʵ���һЩ����ʹ��������
#pragma warning(disable:4505) // δ���õı��غ������Ƴ�(�ȼ� 4)
#pragma warning(disable:4710) // ����δ����(�ȼ� 4)
#endif

#ifndef _DEBUG
#include <_msvcrt.h>
#endif 

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <Windows.h>

// Ϊ����ͨ�������õľ���
#if _MSC_VER >= 1200
#pragma warning(push)
// ΢��SDK���ڵľ���
#pragma warning(disable:4365) // �з���/�޷��Ų�ƥ��(�ȼ� 4)
#pragma warning(disable:4820) // �ֽ������������ݳ�Ա��(�ȼ� 4)
#pragma warning(disable:4986) // �쳣�淶��ǰ���������ƥ��(MSDNδ�������漶��Ĭ�Ͻ��øþ���)
#endif

#include <comdef.h>

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
