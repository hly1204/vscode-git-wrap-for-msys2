#include "set_cmd.h"

BOOL cmd_contains_rev_parse(INT argc, TCHAR *argv[])
{
    for (INT i = 1; i < argc; ++i)
    {
        if (_tcscmp(argv[i], TEXT("rev-parse")) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL set_cmd(PTCHAR dest, DWORD dwDestSize, INT argc, TCHAR *argv[])
{
    if (FAILED(StringCchCopy(dest, dwDestSize, GIT_PATH)))
    {
        fprintf(stderr, "Error copy strings.\n");
        return FALSE;
    }

    for (int i = 1; i < argc; ++i)
    {
        if (FAILED(StringCchCat(dest, dwDestSize, TEXT(" "))))
        {
            fprintf(stderr, "Error concatenating strings.\n");
            return FALSE;
        }
        if (FAILED(StringCchCat(dest, dwDestSize, argv[i])))
        {
            fprintf(stderr, "Error concatenating strings.\n");
            return FALSE;
        }
    }

    return TRUE;
}
