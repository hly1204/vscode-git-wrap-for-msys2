#include "set_env.h"

BOOL set_env(VOID)
{
    TCHAR path[CMD_BUFSIZE];
    DWORD dwLen, dwRet, dwErr;

    if (FAILED(StringCchCopy(path, _countof(path), USR_BIN_PATH)) ||
        FAILED(StringCchCat(path, _countof(path), TEXT(";"))))
    {
        fprintf(stderr, "Out of memory.\n");
        return FALSE;
    }

    dwLen = _tcslen(path);
    dwRet = GetEnvironmentVariable(TEXT("PATH"), path + dwLen, _countof(path) - dwLen);
    if (dwRet == 0)
    {
        dwErr = GetLastError();
        if (dwErr == ERROR_ENVVAR_NOT_FOUND)
        {
            fprintf(stderr, "Environment variable does not exist.\n");
        }
        return FALSE;
    }
    else if (_countof(path) - dwLen < dwRet)
    {
        fprintf(stderr, "Out of memory.\n");
        return FALSE;
    }

    if (!SetEnvironmentVariable(TEXT("PATH"), path))
    {
        fprintf(stderr, "SetEnvironmentVariable failed (%ld).\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}
