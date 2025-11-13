#include "get_exe_path.h"

BOOL get_exe_path(PTCHAR dest, DWORD dwDestSize)
{
    DWORD dwRet = GetModuleFileName(NULL, dest, dwDestSize);
    DWORD dwErr;

    if (dwRet == 0)
    {
        fprintf(stderr, "GetModuleFileName failed (%ld).\n", GetLastError());
        return FALSE;
    }

    if (dwRet == dwDestSize)
    {
        dwErr = GetLastError();
        if (dwErr == ERROR_INSUFFICIENT_BUFFER)
        {
            fprintf(stderr, "GetModuleFileName failed (%ld, %s).\n", dwErr, "ERROR_INSUFFICIENT_BUFFER");
        }
        else
        {
            fprintf(stderr, "GetModuleFileName failed (%ld).\n", dwErr);
        }
        return FALSE;
    }

    return TRUE;
}
