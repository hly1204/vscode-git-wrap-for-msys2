#ifndef SET_CMD_H
#define SET_CMD_H

#include "pch.h"

BOOL cmd_contains_string(INT argc, TCHAR *argv[], LPCTSTR str);
BOOL set_cmd(PTCHAR dest, DWORD dwDestSize, INT argc, TCHAR *argv[]);

#endif
