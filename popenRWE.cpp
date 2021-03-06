/**
 * Copyright 2009-2010 Bart Trojanowski <bart@jukie.net>
 * Licensed under GPLv2, or later, at your choosing.
 *
 */

#include <signal.h>

#include "popenRWE.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief bidirectional popen() call.
 * The caller passes in an array of three integers (rwepipe), on successful
 *        execution it can then write to element 0 (stdin of exe), and read from
 *        element 1 (stdout) and 2 (stderr).
 *
 * @param rwepipe - int array of size three
 * @param command - program to run
 *
 * @return  pid or -1 on error
 */
int popenRWE(int *rwepipe, const char *command)
{
    if (!rwepipe || !command) {
        return -1;
    }

    int in[2];
    int out[2];
    int err[2];
    int pid;
    int rc;

    rc = ::pipe(in);
    if (rc<0) {
        goto error_in;
    }

    rc = ::pipe(out);
    if (rc<0) {
        goto error_out;
    }

    rc = ::pipe(err);
    if (rc<0) {
        goto error_err;
    }

    //! http://stackoverflow.com/questions/18437779/do-i-need-to-do-anything-with-a-sigchld-handler-if-i-am-just-using-wait-to-wai
    // signal(SIGCHLD, SIG_IGN);
    // 设置上句之后，子进程的退出，会导致父进程也退出？
    // 太奇怪了

    pid = ::fork();

    if (pid > 0) { /* parent */
        ::close(in[0]);
        ::close(out[1]);
        ::close(err[1]);
        rwepipe[0] = in[1];
        rwepipe[1] = out[0];
        rwepipe[2] = err[0];
        return pid;
    }
    else if (pid == 0) { /* child */
        ::close(in[1]);
        ::close(out[0]);
        ::close(err[0]);
        ::close(0);
        if(!dup(in[0])) {
            ;
        }
        ::close(1);
        if(!dup(out[1])) {
            ;
        }
        ::close(2);
        if(!dup(err[1])) {
            ;
        }

        execl( "/bin/sh", "sh", "-c", command, NULL );
        _exit(1);
    }
    else {
        goto error_fork;
    }

    return pid;

error_fork:
    ::close(err[0]);
    ::close(err[1]);

error_err:
    ::close(out[0]);
    ::close(out[1]);

error_out:
    ::close(in[0]);
    ::close(in[1]);

error_in:
    return -1;
}

int pcloseRWE(int pid, int *rwepipe)
{
    if (!rwepipe) {
        return -1;
    }

    ::close(rwepipe[0]);
    ::close(rwepipe[1]);
    ::close(rwepipe[2]);

    int status;
    int rc = ::waitpid(pid, &status, 0);

    if (rc == -1) {
        status = -1;
    }

    return status;
}

#ifdef __cplusplus
}
#endif
