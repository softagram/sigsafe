/* $Id$ */

#include <sys/ptrace.h>
#include <stdlib.h>
#include "race_checker.h"

void trace_attach(pid_t pid) {
    error_wrap(ptrace(PTRACE_ATTACH, pid, NULL, NULL),
               "ptrace(PTRACE_ATTACH, ...)", ERRNO);
}

void trace_step(pid_t pid) {
    error_wrap(ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL),
               "ptrace(PTRACE_SINGLESTEP, ...)", ERRNO);
}

void trace_detach(pid_t pid, int signum) {
    error_wrap(ptrace(PTRACE_DETACH, pid, NULL, (void*) signum),
               "ptrace(PTRACE_DETACH, ...)", ERRNO);
}
