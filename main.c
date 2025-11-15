#include "call_git.h"
#include "get_exe_path.h"
#include "pch.h"
#include "set_cmd.h"
#include "set_env.h"

INT _tmain(INT argc, TCHAR *argv[])
{
    TCHAR cmd[CMD_BUFSIZE];
#ifndef NDEBUG
    TCHAR exe_path[BUFSIZE];
    FILE *f;
#endif
    DWORD dwExitCode;

    set_cmd(cmd, _countof(cmd), argc, argv);

#ifndef NDEBUG
    if (get_exe_path(exe_path, _countof(exe_path)) &&
        SUCCEEDED(StringCchCat(exe_path, _countof(exe_path), TEXT(".log"))))
    {
#ifdef UNICODE
        f = _wfopen(exe_path, L"a");
#else
        f = fopen(exe_path, "a");
#endif
        if (f != NULL)
        {
#ifdef UNICODE
            fwprintf(f, L"[GitCall] %ls\n", cmd);
#else
            fprintf(f, "[GitCall] %s\n", cmd);
#endif
            fflush(f);
            fclose(f);
        }
    }
#endif

    /* set_env() is required, otherwise git may failed to use other *.exe */
    if (!set_env())
    {
        exit(EXIT_FAILURE);
    }

    if (cmd_contains_string(argc, argv, TEXT("rev-parse")))
    {
        if (!call_git_pipe_cygpath(cmd, &dwExitCode))
        {
            exit(EXIT_FAILURE);
        }
    }
    else if (cmd_contains_string(argc, argv, TEXT("check-ignore")) && cmd_contains_string(argc, argv, TEXT("-v")) &&
             cmd_contains_string(argc, argv, TEXT("-z")) && cmd_contains_string(argc, argv, TEXT("--stdin")))
    {
        if (!call_git_check_ignore_v_z_stdin(&dwExitCode))
        {
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (!call_git(cmd, &dwExitCode))
        {
            exit(EXIT_FAILURE);
        }
    }

    return (int)dwExitCode;
}
