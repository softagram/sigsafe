/** @file
 * Common definitions for race checker.
 * @legal
 * Copyright &copy; 2004 Scott Lamb &lt;slamb@slamb.org&gt;.
 * This file is part of sigsafe, which is released under the MIT license.
 * @version         $Id$
 * @author          Scott Lamb &lt;slamb@slamb.org&gt;
 */

#ifndef RACECHECKER_H
#define RACECHECKER_H

#include <sys/types.h>      /* for pid_t */
#include <signal.h>         /* for sig_atomic_t */
#include <setjmp.h>         /* for sigsetjmp */

/** Type of error return; used by error_wrap */
enum error_return_type {
    DIRECT,   /**< pthread functions; return 0 or Exxx */
    NEGATIVE, /**< sigsafe functions; return >= 0 or -Exxx */
    ERRNO     /**< old-school functions; return >= 0 or -1, Exxx in errno */
};

/** Aborts if retval/type/errno indicate error. */
int error_wrap(int retval, const char *fname, enum error_return_type type);

enum run_result {
    /* skip 0 */
    INTERRUPTED = 1,
    NORMAL,
    WEIRD
};

/**
 * @defgroup trace Platform-specific process tracing functions
 */
/*@{*/
void trace_me(void);
void trace_attach(pid_t);
void trace_step(pid_t, int);
void trace_continue(pid_t, int);
/*@}*/

/**
 * @defgroup races_generic Generic routines
 */
/*@{*/
extern volatile sig_atomic_t signal_received;
extern volatile sig_atomic_t jump_is_safe;
extern sigjmp_buf env;

void install_safe(void*);
void install_unsafe(void*);

enum run_result do_install_safe(void*);
/*@}*/

/**
 * @defgroup races_io Input/output system calls
 */
/*@{*/
void* create_pipe(void);
void cleanup_pipe(void*);
void do_sigsafe_select_read_child_setup(void*);

enum run_result do_sigsafe_read(void*);
enum run_result do_sigsafe_select_read(void*);
enum run_result do_racebefore_read(void*);
enum run_result do_raceafter_read(void*);
void nudge_read(void*);
/*@}*/

#endif /* !RACECHECKER_H */
