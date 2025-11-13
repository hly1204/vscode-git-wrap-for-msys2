#ifndef CALL_GIT_H
#define CALL_GIT_H

#include "pch.h"

/**
 * @brief git [OPTIONS]
 */
BOOL call_git(PTCHAR cmd, LPDWORD lpExitCode);
/**
 * @brief git [OPTIONS] | cygpath -wf -
 */
BOOL call_git_pipe_cygpath(PTCHAR cmd, LPDWORD lpExitCode);

#endif
