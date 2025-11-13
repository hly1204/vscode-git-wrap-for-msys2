#include "call_git.h"

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
    HANDLE hGit_STDOUT_READ = NULL;
    HANDLE hGit_STDOUT_WRITE = NULL;
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
