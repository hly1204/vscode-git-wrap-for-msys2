#ifdef UNICODE
#define _UNICODE 1
#endif

/* clang-format off */
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
/* clang-format on */

#ifndef USR_BIN_PATH
#define USR_BIN_PATH TEXT("c:\\msys64\\usr\\bin")
#endif

#define GIT_PATH (USR_BIN_PATH TEXT("\\git.exe"))
#define CYGPATH_PATH (USR_BIN_PATH TEXT("\\cygpath.exe"))
#define ADDITIONAL_PATH (TEXT(";") USR_BIN_PATH)

/**
 * @see https://learn.microsoft.com/en-us/troubleshoot/windows-client/shell-experience/command-line-string-limitation
 */
#define BUFSIZE (8191 + 1)
#define CMD_BUFSIZE (32767 + 1)
