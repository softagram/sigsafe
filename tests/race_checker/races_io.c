/**
 * @legal
 * Copyright &copy; 2004 Scott Lamb &lt;slamb@slamb.org&gt;.
 * This file is released under the MIT license.
 * @version         $Id$
 * @author          Scott Lamb &lt;slamb@slamb.org&gt;
 */

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include <sigsafe.h>
#include "race_checker.h"

enum pipe_half {
    READ = 0,
    WRITE
};

void* create_pipe() {
    void *test_data = NULL;

    test_data = malloc(sizeof(int)*2);
    assert(test_data != NULL);
    error_wrap(pipe((int*) test_data), "pipe", ERRNO);
    return test_data;
}

void cleanup_pipe(void *test_data) {
    int *mypipe = (int*) test_data;
    error_wrap(close(mypipe[READ]), "close", ERRNO);
    error_wrap(close(mypipe[WRITE]), "close", ERRNO);
    free(test_data);
}

enum run_result do_safe_read(void *test_data) {
    char c;
    int *mypipe = (int*) test_data;
    int retval;

    retval = sigsafe_read(mypipe[READ], &c, sizeof(char));
    if (retval == -EINTR) {
        return INTERRUPTED;
    } else if (retval == 1) {
        return NORMAL;
    } else {
        return WEIRD;
    }
}

enum run_result do_unsafe_read(void *test_data) {
    char c;
    int *mypipe = (int*) test_data;
    int retval;

    if (signal_received) {
        return INTERRUPTED;
    }
    retval = read(mypipe[READ], &c, sizeof(char));
    if (retval == -1 && errno == EINTR) {
        return INTERRUPTED;
    } else if (retval == 1) {
        return NORMAL;
    } else {
        return WEIRD;
    }
}

void nudge_read(void *test_data) {
    char c = 26;
    int *mypipe = (int*) test_data;
    error_wrap(write(mypipe[WRITE], &c, sizeof(char)), "write", ERRNO);
}