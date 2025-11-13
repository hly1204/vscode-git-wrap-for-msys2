#include "set_env.h"

BOOL set_env(VOID)
{
    PTCHAR oldPath, newPath;
    DWORD dwRet, dwErr;

    oldPath = (PTCHAR)malloc(BUFSIZE * sizeof(TCHAR));
    if (oldPath == NULL)
    {
        fprintf(stderr, "Out of memory.\n");
        return FALSE;
    }

    dwRet = GetEnvironmentVariable(TEXT("PATH"), oldPath, BUFSIZE);
    if (dwRet == 0)
    {
        dwErr = GetLastError();
        if (dwErr == ERROR_ENVVAR_NOT_FOUND)
        {
            fprintf(stderr, "Environment variable does not exist.\n");
        }
        free(oldPath);
        return FALSE;
    }
    else if (BUFSIZE < dwRet)
    {
        newPath = (PTCHAR)realloc(oldPath, (dwRet + _countof(ADDITIONAL_PATH)) * sizeof(TCHAR));
        if (newPath == NULL)
        {
            fprintf(stderr, "Out of memory.\n");
            free(oldPath);
            return FALSE;
        }
        oldPath = NULL;
        dwRet = GetEnvironmentVariable(TEXT("PATH"), newPath, dwRet + _countof(ADDITIONAL_PATH));
        if (dwRet == 0)
        {
            fprintf(stderr, "GetEnvironmentVariable failed (%ld).\n", GetLastError());
            free(newPath);
            return FALSE;
        }
        else
        {
            if (FAILED(StringCchCat(newPath, dwRet + _countof(ADDITIONAL_PATH), ADDITIONAL_PATH)))
            {
                fprintf(stderr, "Error concatenating strings.\n");
                free(newPath);
                return FALSE;
            }
        }
    }
    else
    {
        newPath = (PTCHAR)realloc(oldPath, (dwRet + _countof(ADDITIONAL_PATH)) * sizeof(TCHAR));
        if (newPath == NULL)
        {
            fprintf(stderr, "Out of memory.\n");
            free(oldPath);
            return FALSE;
        }
        oldPath = NULL;
        if (FAILED(StringCchCat(newPath, dwRet + _countof(ADDITIONAL_PATH), ADDITIONAL_PATH)))
        {
            fprintf(stderr, "Error concatenating strings.\n");
            free(newPath);
            return FALSE;
        }
    }

    if (!SetEnvironmentVariable(TEXT("PATH"), newPath))
    {
        fprintf(stderr, "SetEnvironmentVariable failed (%ld).\n", GetLastError());
        free(newPath);
        return FALSE;
    }

    free(newPath);
    return TRUE;
}
