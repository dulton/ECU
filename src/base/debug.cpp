#include "debug.h"
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <DbgHelp.h>
#include "version.h"
#include <Strsafe.h>
#include <stdlib.h>
#include <tlhelp32.h>

char DEXIT_BUFFER[4096];

void SetThreadName(LPCSTR szThreadName, const DWORD dwThreadID)
{
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = szThreadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  __try
  {
    RaiseException(0x406D1388, 0,
                   sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
  }
  __except(EXCEPTION_CONTINUE_EXECUTION) {}
}
char* DExitProcess(char* file, char* func_signature, int line) {
  sprintf_s(DEXIT_BUFFER, "\n\n%s \n%s || LINE:%d\n\n", file, func_signature,
            line);
  OutputDebugStringA(
      DEXIT_BUFFER); // func_signature outputs in char *, no auto coversion
  ExitProcess(line);
}


void write_log(wchar_t *text, DWORD error, char *file, int line)
{
  HANDLE log = CreateFile(L"log.txt", GENERIC_READ|GENERIC_WRITE, 0, NULL,
                          OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  DWORD bytes_written;
  SYSTEMTIME date;
  GetLocalTime(&date);
  
  LPTSTR error_text = NULL;
  
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM
                |FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&error_text,
                0, NULL);
  
  SetFilePointer(log, 0, NULL, FILE_END);
  wchar_t text_final[4096];
  
  wchar_t file_wide[248];
  size_t char_converted;
  mbstowcs_s(&char_converted, file_wide, 248, file, _TRUNCATE);

  if (error == 0) {
    swprintf_s(text_final, 4096,
               L"[%d/%d/%d %d:%d:%d] %s[%d]: %s\r\n",
               date.wDay, date.wMonth, date.wYear,
               date.wHour, date.wMinute, date.wSecond, file_wide,
               line, text);
  }
  
  else {
    swprintf_s(text_final, 4096,
               L"[%d/%d/%d %d:%d:%d] %s[%d]: %s\r\n",
               date.wDay, date.wMonth, date.wYear,
               date.wHour, date.wMinute, date.wSecond, file_wide,
               line, error_text);
  }
  
  WriteFile(log, text_final, wcslen(text_final)*sizeof(wchar_t), &bytes_written, NULL);
  
  CloseHandle(log);
  HeapFree(GetProcessHeap(), NULL, error_text);
}

void write_log(wchar_t *text)
{
  HANDLE log = CreateFile(L"log.txt", GENERIC_READ|GENERIC_WRITE, 0, NULL,
                          OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  SYSTEMTIME date;
  DWORD bytes_written;
  
  SetFilePointer(log, 0, NULL, FILE_END);
  
  
  GetLocalTime(&date);
  
  wchar_t text_final[4096];
  
  swprintf_s(text_final, 4096,
               L"[%d/%d/%d %d:%d:%d]: %s\r\n",
               date.wDay, date.wMonth, date.wYear,
               date.wHour, date.wMinute, date.wSecond, text);
  
  WriteFile(log, text, wcslen(text)*sizeof(wchar_t), &bytes_written, NULL);
  CloseHandle(log);
}

void write_log(const char *text)
{
  HANDLE log = CreateFile(L"log.txt", GENERIC_READ|GENERIC_WRITE, 0, NULL,
                          OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  SYSTEMTIME date;
  DWORD bytes_written;
  
  SetFilePointer(log, 0, NULL, FILE_END);
    
  GetLocalTime(&date);
  
  size_t char_converted;
  wchar_t text_wide[4096];
  mbstowcs_s(&char_converted, text_wide, 4096, text, _TRUNCATE);

  wchar_t text_final[4096];
    
  swprintf_s(text_final, 4096,
               L"[%d/%d/%d %d:%d:%d]: %s\r\n",
               date.wDay, date.wMonth, date.wYear,
               date.wHour, date.wMinute, date.wSecond, text_wide);
  
  WriteFile(log, text_final, wcslen(text_final)*sizeof(wchar_t), &bytes_written, NULL);
  CloseHandle(log);
}

BOOL close_window(HWND window, LPARAM param)
{
  if (IsWindow(window)) {
    SendMessage(window, WM_QUIT, 0, 0);
    PostMessage(window, WM_CLOSE, 0, 0);
    ExitProcess(1);
    return true;
  }
  return false;
}

BOOL ListProcessThreads(DWORD dwOwnerPID)
{
  HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
  THREADENTRY32 te32;

  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap == INVALID_HANDLE_VALUE)
    return false;

  te32.dwSize = sizeof(THREADENTRY32);
  if (!Thread32First(hThreadSnap, &te32)) {
    // write error to log
    CloseHandle(hThreadSnap);
    return false;
  }

  do
  {
    if (te32.th32OwnerProcessID == dwOwnerPID) {
      EnumThreadWindows(dwOwnerPID, WNDENUMPROC (close_window), 0);
      PostThreadMessage(te32.th32ThreadID, WM_CLOSE, 0, 0);
    }
  } while (Thread32Next(hThreadSnap, &te32));

  CloseHandle(hThreadSnap);
  return true;
  
}

void CreateMiniDump(int exit)
{
  size_t char_converted;
  wchar_t filename[512];
 
  swprintf_s(filename, 512, L"%s-%s_%d.dmp", _T(ECU_BUILD_COMMIT_SHORT),
             _T(ECU_BUILD_BRANCH), exit);

  HANDLE file = CreateFile(filename, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  
  if ((file != NULL) && (file != INVALID_HANDLE_VALUE)) {
    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = 0;
    mdei.ClientPointers = 0;
    MINIDUMP_TYPE mdt = MiniDumpNormal;
    BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, mdt,
                                0, 0, 0);
    if (!rv)
      // do something
    CloseHandle(file);
  }

  if (exit) {
    ListProcessThreads(GetCurrentProcessId());
  }
}
