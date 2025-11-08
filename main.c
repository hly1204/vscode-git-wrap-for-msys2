#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>

#define USR_BIN_PATH TEXT("c:\\msys64\\usr\\bin")

#define GIT_PATH (USR_BIN_PATH TEXT("\\git.exe"))
#define CYGPATH_PATH (USR_BIN_PATH TEXT("\\cygpath.exe"))
#define ADDITIONAL_PATH (TEXT(";") USR_BIN_PATH)
#define BUFSIZE 4097
#define CMD_BUFSIZE 4097

static BOOL cmd_contains_rev_parse(int argc, TCHAR *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (_tcscmp(argv[i], TEXT("rev-parse")) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

static void set_cmd(PTCHAR dest, size_t destSize, int argc, TCHAR *argv[])
{
    if (FAILED(StringCchCopy(dest, destSize, GIT_PATH))) {
        fprintf(stderr, "Error copy strings.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; ++i) {
        if (FAILED(StringCchCat(dest, destSize, TEXT(" ")))) {
            fprintf(stderr, "Error concatenating strings.\n");
            exit(EXIT_FAILURE);
        }
        if (FAILED(StringCchCat(dest, destSize, argv[i]))) {
            fprintf(stderr, "Error concatenating strings.\n");
            exit(EXIT_FAILURE);
        }
    }
}

static void set_env(void)
{
    PTCHAR oldPath, newPath;
    DWORD dwRet, dwErr;

    oldPath = (PTCHAR)malloc(BUFSIZE * sizeof(TCHAR));
    if (oldPath == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    dwRet = GetEnvironmentVariable(TEXT("PATH"), oldPath, BUFSIZE);
    if (dwRet == 0) {
        dwErr = GetLastError();
        if (dwErr == ERROR_ENVVAR_NOT_FOUND) {
            fprintf(stderr, "Environment variable does not exist.\n");
        }
        free(oldPath);
        exit(EXIT_FAILURE);
    } else if (BUFSIZE < dwRet) {
        newPath = (PTCHAR)realloc(oldPath, (dwRet + _countof(ADDITIONAL_PATH)) * sizeof(TCHAR));
        if (newPath == NULL) {
            fprintf(stderr, "Out of memory.\n");
            free(oldPath);
            exit(EXIT_FAILURE);
        }
        oldPath = NULL;
        dwRet = GetEnvironmentVariable(TEXT("PATH"), newPath, dwRet + _countof(ADDITIONAL_PATH));
        if (dwRet == 0) {
            fprintf(stderr, "GetEnvironmentVariable failed (%d).\n", (int)GetLastError());
            free(newPath);
            exit(EXIT_FAILURE);
        } else {
            if (FAILED(StringCchCat(newPath, dwRet + _countof(ADDITIONAL_PATH), ADDITIONAL_PATH))) {
                fprintf(stderr, "Error concatenating strings.\n");
                free(newPath);
                exit(EXIT_FAILURE);
            }
        }
    } else {
        newPath = (PTCHAR)realloc(oldPath, (dwRet + _countof(ADDITIONAL_PATH)) * sizeof(TCHAR));
        if (newPath == NULL) {
            fprintf(stderr, "Out of memory.\n");
            free(oldPath);
            exit(EXIT_FAILURE);
        }
        oldPath = NULL;
        if (FAILED(StringCchCat(newPath, dwRet + _countof(ADDITIONAL_PATH), ADDITIONAL_PATH))) {
            fprintf(stderr, "Error concatenating strings.\n");
            free(newPath);
            exit(EXIT_FAILURE);
        }
    }

    if (!SetEnvironmentVariable(TEXT("PATH"), newPath)) {
        fprintf(stderr, "SetEnvironmentVariable failed (%d).\n", (int)GetLastError());
        free(newPath);
        exit(EXIT_FAILURE);
    }

    free(newPath);
}

static void create_git_process(PTCHAR cmd)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD dwFlags = 0;

#ifdef UNICODE
    dwFlags = CREATE_UNICODE_ENVIRONMENT;
#endif

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Passing all arguments to child process
    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, dwFlags, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "Error creating process.\n");
        exit(EXIT_FAILURE);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

static void create_git_process2(PTCHAR cmd)
{
    STARTUPINFO si_Git, si_Cygpath;
    PROCESS_INFORMATION pi_Git, pi_Cygpath;
    DWORD dwFlags = 0;
    HANDLE hGitStd_OUT_Rd = NULL;
    HANDLE hGitStd_OUT_Wr = NULL;
    SECURITY_ATTRIBUTES seAttr;

    ZeroMemory(&seAttr, sizeof(seAttr));
    seAttr.nLength = sizeof(seAttr);
    seAttr.bInheritHandle = TRUE;
    seAttr.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&hGitStd_OUT_Rd, &hGitStd_OUT_Wr, &seAttr, 0)) {
        fprintf(stderr, "CreatePipe failed (%d).\n", (int)GetLastError());
        exit(EXIT_FAILURE);
    }

    if (!SetHandleInformation(hGitStd_OUT_Wr, HANDLE_FLAG_INHERIT, 0)) {
        fprintf(stderr, "SetHandleInformation failed (%d).\n", (int)GetLastError());
        CloseHandle(hGitStd_OUT_Rd);
        CloseHandle(hGitStd_OUT_Wr);
        exit(EXIT_FAILURE);
    }

    ZeroMemory(&si_Cygpath, sizeof(si_Cygpath));
    si_Cygpath.cb = sizeof(si_Cygpath);
    si_Cygpath.hStdInput = hGitStd_OUT_Rd;
    // Cannot use null here.
    si_Cygpath.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si_Cygpath.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si_Cygpath.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi_Cygpath, sizeof(pi_Cygpath));

    if (!CreateProcess(NULL, (USR_BIN_PATH TEXT("\\cygpath.exe -wf -")), NULL, NULL, TRUE, dwFlags,
                       NULL, NULL, &si_Cygpath, &pi_Cygpath)) {
        fprintf(stderr, "Error creating process.\n");
        CloseHandle(hGitStd_OUT_Rd);
        CloseHandle(hGitStd_OUT_Wr);
        exit(EXIT_FAILURE);
    }

    if (!SetHandleInformation(hGitStd_OUT_Wr, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        fprintf(stderr, "SetHandleInformation failed (%d).\n", (int)GetLastError());
        CloseHandle(hGitStd_OUT_Rd);
        CloseHandle(hGitStd_OUT_Wr);
        WaitForSingleObject(pi_Cygpath.hProcess, INFINITE);
        CloseHandle(pi_Cygpath.hProcess);
        CloseHandle(pi_Cygpath.hThread);
        exit(EXIT_FAILURE);
    }

    if (!SetHandleInformation(hGitStd_OUT_Rd, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        fprintf(stderr, "SetHandleInformation failed (%d).\n", (int)GetLastError());
        CloseHandle(hGitStd_OUT_Rd);
        CloseHandle(hGitStd_OUT_Wr);
        WaitForSingleObject(pi_Cygpath.hProcess, INFINITE);
        CloseHandle(pi_Cygpath.hProcess);
        CloseHandle(pi_Cygpath.hThread);
        exit(EXIT_FAILURE);
    }

    ZeroMemory(&si_Git, sizeof(si_Git));
    si_Git.cb = sizeof(si_Git);
    si_Git.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si_Git.hStdOutput = hGitStd_OUT_Wr;
    si_Git.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si_Git.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi_Git, sizeof(pi_Git));

#ifdef UNICODE
    dwFlags = CREATE_UNICODE_ENVIRONMENT;
#endif

    // Passing all arguments to child process
    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, dwFlags, NULL, NULL, &si_Git, &pi_Git)) {
        fprintf(stderr, "Error creating process.\n");
        CloseHandle(hGitStd_OUT_Rd);
        CloseHandle(hGitStd_OUT_Wr);
        WaitForSingleObject(pi_Cygpath.hProcess, INFINITE);
        CloseHandle(pi_Cygpath.hProcess);
        CloseHandle(pi_Cygpath.hThread);
        exit(EXIT_FAILURE);
    }

    WaitForSingleObject(pi_Git.hProcess, INFINITE);
    CloseHandle(pi_Git.hProcess);
    CloseHandle(pi_Git.hThread);

    CloseHandle(hGitStd_OUT_Wr);
    CloseHandle(hGitStd_OUT_Rd);

    WaitForSingleObject(pi_Cygpath.hProcess, INFINITE);
    CloseHandle(pi_Cygpath.hProcess);
    CloseHandle(pi_Cygpath.hThread);
}

int _tmain(int argc, TCHAR *argv[])
{
    TCHAR cmd[CMD_BUFSIZE];

    ZeroMemory(&cmd, sizeof(cmd));
    set_cmd(cmd, _countof(cmd), argc, argv);

    FILE *f = fopen("C:\\msys64\\home\\hly\\cmd.txt", "a");
    fprintf(f, "%s\r\n", cmd);
    fclose(f);

    set_env();
    if (!cmd_contains_rev_parse(argc, argv)) {
        create_git_process(cmd);
    } else {
        create_git_process2(cmd);
    }
    exit(EXIT_SUCCESS);
}
