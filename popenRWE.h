/**
 * Copyright 2009-2010 Bart Trojanowski <bart@jukie.net>
 * Licensed under GPLv2, or later, at your choosing.
 *
 * bidirectional popen() call
 *
 * The caller passes in an array of three integers (rwepipe), on successful
 * execution it can then write to element 0 (stdin of exe), and read from
 * element 1 (stdout) and 2 (stderr).
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int popenRWE(int *rwepipe, const char *command);
int pcloseRWE(int pid, int *rwepipe);

#ifdef __cplusplus
}
#endif
