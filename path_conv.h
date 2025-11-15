#ifndef PATH_CONV_H
#define PATH_CONV_H

#include "pch.h"

BOOL win_path_to_unix_path(PCHAR buffer, DWORD dwBufferSize, LPCTSTR path);
BOOL unix_path_to_win_path(PCHAR buffer, DWORD dwBufferSize, LPCTSTR path);

#endif
