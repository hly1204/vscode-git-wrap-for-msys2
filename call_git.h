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

/**
 * @brief git check-ignore -v -z --stdin
 * @details git check-ignore -v -z --stdin | [...]
 */
BOOL call_git_check_ignore_v_z_stdin(LPDWORD lpExitCode);

#endif
