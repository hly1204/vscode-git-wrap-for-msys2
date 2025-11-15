#include "call_git.h"
#include "path_conv.h"

static INT count_null(PCSTR begin, PCSTR end)
{
    PCTSTR i;
    INT iCount = 0;

    for (i = begin; i != end; ++i)
    {
        if (*i == '\0')
        {
            ++iCount;
        }
    }

    return iCount;
}

BOOL call_git(PTCHAR cmd, LPDWORD lpExitCode)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD dwFlags = 0;

#ifdef UNICODE
    dwFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (!CreateProcess(GIT_PATH, cmd, NULL, NULL, FALSE, dwFlags, NULL, NULL, &si, &pi))
    {
        fprintf(stderr, "Error creating process.\n");
        return FALSE;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    if (!GetExitCodeProcess(pi.hProcess, lpExitCode))
    {
        fprintf(stderr, "Error GetExitCodeProcess (%ld).\n", GetLastError());
        *lpExitCode = 0;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return TRUE;
}

BOOL call_git_pipe_cygpath(PTCHAR cmd, LPDWORD lpExitCode)
{
    STARTUPINFO siGit, siCygpath;
    PROCESS_INFORMATION piGit, piCygpath;
    DWORD dwFlags = 0;
    HANDLE hGit_STDOUT_READ;
    HANDLE hGit_STDOUT_WRITE;
    SECURITY_ATTRIBUTES seAttr;

#ifdef UNICODE
    dwFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif

    ZeroMemory(&seAttr, sizeof(seAttr));
    seAttr.nLength = sizeof(seAttr);
    seAttr.bInheritHandle = TRUE;
    seAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&hGit_STDOUT_READ, &hGit_STDOUT_WRITE, &seAttr, 0))
    {
        fprintf(stderr, "CreatePipe failed (%ld).\n", GetLastError());
        return FALSE;
    }

    if (!SetHandleInformation(hGit_STDOUT_WRITE, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "SetHandleInformation failed (%ld).\n", GetLastError());
        CloseHandle(hGit_STDOUT_READ);
        CloseHandle(hGit_STDOUT_WRITE);
        return FALSE;
    }

    ZeroMemory(&siCygpath, sizeof(siCygpath));
    siCygpath.cb = sizeof(siCygpath);
    siCygpath.hStdInput = hGit_STDOUT_READ;
    siCygpath.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siCygpath.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siCygpath.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, (USR_BIN_PATH TEXT("\\cygpath.exe -wf -")), NULL, NULL, TRUE, dwFlags, NULL, NULL,
                       &siCygpath, &piCygpath))
    {
        fprintf(stderr, "Error creating process.\n");
        CloseHandle(hGit_STDOUT_READ);
        CloseHandle(hGit_STDOUT_WRITE);
        return FALSE;
    }

    CloseHandle(hGit_STDOUT_READ);

    if (!SetHandleInformation(hGit_STDOUT_WRITE, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
    {
        fprintf(stderr, "SetHandleInformation failed (%ld).\n", GetLastError());
        CloseHandle(hGit_STDOUT_WRITE);
        WaitForSingleObject(piCygpath.hProcess, INFINITE);
        CloseHandle(piCygpath.hProcess);
        CloseHandle(piCygpath.hThread);
        return FALSE;
    }

    ZeroMemory(&siGit, sizeof(siGit));
    siGit.cb = sizeof(siGit);
    siGit.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siGit.hStdOutput = hGit_STDOUT_WRITE;
    siGit.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siGit.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, dwFlags, NULL, NULL, &siGit, &piGit))
    {
        fprintf(stderr, "Error creating process.\n");
        CloseHandle(hGit_STDOUT_WRITE);
        WaitForSingleObject(piCygpath.hProcess, INFINITE);
        CloseHandle(piCygpath.hProcess);
        CloseHandle(piCygpath.hThread);
        return FALSE;
    }

    CloseHandle(hGit_STDOUT_WRITE);
    WaitForSingleObject(piGit.hProcess, INFINITE);
    if (!GetExitCodeProcess(piGit.hProcess, lpExitCode))
    {
        fprintf(stderr, "Error GetExitCodeProcess (%ld).\n", GetLastError());
        *lpExitCode = 0;
    }

    CloseHandle(piGit.hProcess);
    CloseHandle(piGit.hThread);

    WaitForSingleObject(piCygpath.hProcess, INFINITE);
    CloseHandle(piCygpath.hProcess);
    CloseHandle(piCygpath.hThread);

    return TRUE;
}

BOOL call_git_check_ignore_v_z_stdin(LPDWORD lpExitCode)
{
    STARTUPINFO siGit;
    PROCESS_INFORMATION piGit;
    DWORD dwFlags = 0;

    HANDLE hGit_STDOUT_READ;
    HANDLE hGit_STDOUT_WRITE;

    SECURITY_ATTRIBUTES seAttr;

    CHAR buffer[CMD_BUFSIZE];
    CHAR path[CMD_BUFSIZE];
    PCHAR pathEnd;
    DWORD iResult;
    DWORD iBytesRead;

    PCHAR bufferEnd = buffer;

    /* <source> <NULL> <linenum> <NULL> <pattern> <NULL> <pathname> <NULL> */
    PCHAR bufferIterator;
    DWORD iPartCount;
    PCHAR partEnd[4];

    DWORD iBytesWrite;

#ifdef UNICODE
    dwFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif

    ZeroMemory(&seAttr, sizeof(seAttr));
    seAttr.nLength = sizeof(seAttr);
    seAttr.bInheritHandle = TRUE;
    seAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&hGit_STDOUT_READ, &hGit_STDOUT_WRITE, &seAttr, 0))
    {
        fprintf(stderr, "CreatePipe failed (%ld).\n", GetLastError());
        return FALSE;
    }

    if (!SetHandleInformation(hGit_STDOUT_READ, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "SetHandleInformation failed (%ld).\n", GetLastError());
        CloseHandle(hGit_STDOUT_READ);
        CloseHandle(hGit_STDOUT_WRITE);
        return FALSE;
    }

    ZeroMemory(&siGit, sizeof(siGit));
    siGit.cb = sizeof(siGit);
    siGit.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siGit.hStdOutput = hGit_STDOUT_WRITE;
    siGit.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siGit.dwFlags |= STARTF_USESTDHANDLES;

    /* Luckily Git accepts a Windows style path from stdin, so we don't need to use cygpath again and again. */
    if (!CreateProcess(NULL, (USR_BIN_PATH TEXT("\\git.exe check-ignore -v -z --stdin")), NULL, NULL, TRUE, dwFlags,
                       NULL, NULL, &siGit, &piGit))
    {
        fprintf(stderr, "Error creating process.\n");
        CloseHandle(hGit_STDOUT_READ);
        CloseHandle(hGit_STDOUT_WRITE);
        return FALSE;
    }

    CloseHandle(hGit_STDOUT_WRITE);
    for (;;)
    {
        iResult = ReadFile(hGit_STDOUT_READ, bufferEnd, buffer + _countof(buffer) - bufferEnd, &iBytesRead, NULL);

        if (!iResult || iBytesRead == 0)
        {
            break;
        }
        bufferEnd += iBytesRead;

        while (count_null(buffer, bufferEnd) >= 4)
        {
            for (iPartCount = 0, bufferIterator = buffer; bufferIterator != bufferEnd && iPartCount < 4;
                 ++bufferIterator)
            {
                if (*bufferIterator == '\0')
                {
                    partEnd[iPartCount++] = bufferIterator;
                }
            }

            if (unix_path_to_win_path(path, _countof(path), partEnd[2] + 1))
            {
                for (bufferIterator = buffer; partEnd[2] + 1 > bufferIterator; bufferIterator += iBytesWrite)
                {
                    iResult = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), bufferIterator,
                                        partEnd[2] + 1 - bufferIterator /* write NULL */, &iBytesWrite, NULL);
                    if (!iResult || iBytesWrite == 0)
                    {
                        break;
                    }
                }

                /* Convert Windows style path to lowercase before ':',
                   because VSCode can only recognize lowercase drive letter. */
                for (bufferIterator = path; *bufferIterator != '\0' && *bufferIterator != ':'; ++bufferIterator)
                {
                    if (*bufferIterator >= 'A' && *bufferIterator <= 'Z')
                    {
                        *bufferIterator -= 'A';
                        *bufferIterator += 'a';
                    }
                    else
                    {
                        break;
                    }
                }

                for (bufferIterator = path, pathEnd = path + _tcslen(path) + 1 /* write NULL */;
                     pathEnd > bufferIterator; bufferIterator += iBytesWrite)
                {
                    iResult = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), bufferIterator, pathEnd - bufferIterator,
                                        &iBytesWrite, NULL);
                    if (!iResult || iBytesWrite == 0)
                    {
                        break;
                    }
                }
            }

            if (bufferEnd == partEnd[3] + 1)
            {
                bufferEnd = buffer;
            }
            else
            {
                memmove(buffer, partEnd[3] + 1, bufferEnd - (partEnd[3] + 1));
                bufferEnd -= bufferEnd - (partEnd[3] + 1);
            }
        }
    }

    WaitForSingleObject(piGit.hProcess, INFINITE);
    if (!GetExitCodeProcess(piGit.hProcess, lpExitCode))
    {
        fprintf(stderr, "Error GetExitCodeProcess (%ld).\n", GetLastError());
        *lpExitCode = 0;
    }
    CloseHandle(piGit.hProcess);
    CloseHandle(piGit.hThread);

    CloseHandle(hGit_STDOUT_READ);

    return TRUE;
}
