#include "path_conv.h"

BOOL win_path_to_unix_path(PCHAR buffer, DWORD dwBufferSize, LPCTSTR path)
{
    HANDLE hCygpath_STD_OUT_READ;
    HANDLE hCygpath_STD_OUT_WRITE;
    SECURITY_ATTRIBUTES seAttr;

    STARTUPINFO siCygpath;
    PROCESS_INFORMATION piCygpath;

    TCHAR cmd[CMD_BUFSIZE];
    DWORD iResult;
    DWORD iBytesRead;

    PCHAR bufferEnd = buffer;

    ZeroMemory(&seAttr, sizeof(seAttr));
    seAttr.nLength = sizeof(seAttr);
    seAttr.bInheritHandle = TRUE;
    seAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hCygpath_STD_OUT_READ, &hCygpath_STD_OUT_WRITE, &seAttr, 0))
    {
        fprintf(stderr, "CreatePipe failed (%lu).\n", GetLastError());
        return FALSE;
    }

    if (!SetHandleInformation(hCygpath_STD_OUT_READ, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "SetHandleInformation failed (%lu).\n", GetLastError());
        CloseHandle(hCygpath_STD_OUT_READ);
        CloseHandle(hCygpath_STD_OUT_WRITE);
        return FALSE;
    }

    ZeroMemory(&siCygpath, sizeof(siCygpath));
    siCygpath.cb = sizeof(siCygpath);
    siCygpath.hStdInput = NULL;
    siCygpath.hStdOutput = hCygpath_STD_OUT_WRITE;
    siCygpath.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siCygpath.dwFlags |= STARTF_USESTDHANDLES;

    /* clang-format off */
    if (FAILED(StringCchCopy(cmd, _countof(cmd), (USR_BIN_PATH TEXT("\\cygpath.exe") TEXT(" -u "))))
        /* Use quoted string for the path argument. */
        || FAILED(StringCchCat(cmd, _countof(cmd), TEXT("\"")))
        || FAILED(StringCchCat(cmd, _countof(cmd), path))
        /* If last character is '\', we should escape it. */
        /* see https://ss64.com/nt/syntax-esc.html */
        || (_tcslen(path) > 0
            && path[_tcslen(path) - 1] == TEXT('\\')
            && FAILED(StringCchCat(cmd, _countof(cmd), TEXT("\\"))))
        || FAILED(StringCchCat(cmd, _countof(cmd), TEXT("\""))))
    {
        CloseHandle(hCygpath_STD_OUT_READ);
        CloseHandle(hCygpath_STD_OUT_WRITE);
        return FALSE;
    }
    /* clang-format on */

    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &siCygpath, &piCygpath))
    {
        fprintf(stderr, "Error creating process.\n");
        CloseHandle(hCygpath_STD_OUT_READ);
        CloseHandle(hCygpath_STD_OUT_WRITE);
        return FALSE;
    }

    CloseHandle(hCygpath_STD_OUT_WRITE);
    for (;;)
    {
        iResult = ReadFile(hCygpath_STD_OUT_READ, bufferEnd, buffer + dwBufferSize - bufferEnd, &iBytesRead, NULL);
        if (!iResult || iBytesRead == 0)
        {
            break;
        }
        bufferEnd += iBytesRead;
    }

    WaitForSingleObject(piCygpath.hProcess, INFINITE);
    CloseHandle(hCygpath_STD_OUT_READ);
    CloseHandle(piCygpath.hProcess);
    CloseHandle(piCygpath.hThread);

    /* Trim whitespace */
    while (--bufferEnd >= buffer && isspace(*bufferEnd))
    {
        *bufferEnd = TEXT('\0');
    }
    if (bufferEnd + 1 >= buffer + dwBufferSize)
    {
        fprintf(stderr, "Out of memory.\n");
        return FALSE;
    }
    else
    {
        *++bufferEnd = TEXT('\0');
    }

    return TRUE;
}

BOOL unix_path_to_win_path(PCHAR buffer, DWORD dwBufferSize, LPCTSTR path)
{
    HANDLE hCygpath_STD_OUT_READ;
    HANDLE hCygpath_STD_OUT_WRITE;
    SECURITY_ATTRIBUTES seAttr;

    STARTUPINFO siCygpath;
    PROCESS_INFORMATION piCygpath;

    TCHAR cmd[CMD_BUFSIZE];
    DWORD iResult;
    DWORD iBytesRead;

    PCHAR bufferEnd = buffer;

    ZeroMemory(&seAttr, sizeof(seAttr));
    seAttr.nLength = sizeof(seAttr);
    seAttr.bInheritHandle = TRUE;
    seAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hCygpath_STD_OUT_READ, &hCygpath_STD_OUT_WRITE, &seAttr, 0))
    {
        fprintf(stderr, "CreatePipe failed (%lu).\n", GetLastError());
        return FALSE;
    }

    if (!SetHandleInformation(hCygpath_STD_OUT_READ, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "SetHandleInformation failed (%lu).\n", GetLastError());
        CloseHandle(hCygpath_STD_OUT_READ);
        CloseHandle(hCygpath_STD_OUT_WRITE);
        return FALSE;
    }

    ZeroMemory(&siCygpath, sizeof(siCygpath));
    siCygpath.cb = sizeof(siCygpath);
    siCygpath.hStdInput = NULL;
    siCygpath.hStdOutput = hCygpath_STD_OUT_WRITE;
    siCygpath.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siCygpath.dwFlags |= STARTF_USESTDHANDLES;

    /* clang-format off */
    if (FAILED(StringCchCopy(cmd, _countof(cmd), (USR_BIN_PATH TEXT("\\cygpath.exe") TEXT(" -w "))))
        /* Use quoted string for the path argument. */
        || FAILED(StringCchCat(cmd, _countof(cmd), TEXT("\"")))
        || FAILED(StringCchCat(cmd, _countof(cmd), path))
        || FAILED(StringCchCat(cmd, _countof(cmd), TEXT("\""))))
    {
        CloseHandle(hCygpath_STD_OUT_READ);
        CloseHandle(hCygpath_STD_OUT_WRITE);
        return FALSE;
    }
    /* clang-format on */

    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &siCygpath, &piCygpath))
    {
        fprintf(stderr, "Error creating process.\n");
        CloseHandle(hCygpath_STD_OUT_READ);
        CloseHandle(hCygpath_STD_OUT_WRITE);
        return FALSE;
    }

    CloseHandle(hCygpath_STD_OUT_WRITE);
    for (;;)
    {
        iResult = ReadFile(hCygpath_STD_OUT_READ, bufferEnd, buffer + dwBufferSize - bufferEnd, &iBytesRead, NULL);
        if (!iResult || iBytesRead == 0)
        {
            break;
        }
        bufferEnd += iBytesRead;
    }

    WaitForSingleObject(piCygpath.hProcess, INFINITE);
    CloseHandle(hCygpath_STD_OUT_READ);
    CloseHandle(piCygpath.hProcess);
    CloseHandle(piCygpath.hThread);

    /* Trim whitespace */
    while (--bufferEnd >= buffer && isspace(*bufferEnd))
    {
        *bufferEnd = TEXT('\0');
    }
    if (bufferEnd + 1 >= buffer + dwBufferSize)
    {
        fprintf(stderr, "Out of memory.\n");
        return FALSE;
    }
    else
    {
        *++bufferEnd = TEXT('\0');
    }

    return TRUE;
}
