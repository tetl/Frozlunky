/* RemoteOps.h */
 
#ifndef REM_OPS_H
#define REM_OPS_H
 
#include <windows.h>
#include <commctrl.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

HMODULE WINAPI GetRemoteModuleHandle(HANDLE hProcess, LPCSTR lpModuleName);
FARPROC WINAPI GetRemoteProcAddress (HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName, UINT Ordinal = 0, BOOL UseOrdinal = FALSE);
 
#endif //REM_OPS_H