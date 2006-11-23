#ifdef _THREAD_SAFE
#include <pthread.h>
#endif
#include <sigsafe.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int tsd;

enum error_return_type {
    DIRECT,         /**< pthread functions */
    NEGATIVE,       /**< sigsafe functions */
    ERRNO           /**< old-school functions */
};

int
error_wrap(int retval, const char *funcname, enum error_return_type type)
{
    if (type == ERRNO && retval < 0) {
        fprintf(stderr, "%s returned %d (errno==%d) (%s)\n",
                funcname, retval, errno, strerror(errno));
    } else if (type == DIRECT && retval != 0) {
        fprintf(stderr, "%s returned %d (%s)\n",
                funcname, retval, strerror(retval));
    } else if (type == NEGATIVE && retval < 0) {
        fprintf(stderr, "%s returned %d (%s)\n",
                funcname, retval, strerror(-retval));
    }
    return retval;
}

/*
 * These macros are used by various syscalls as a sanity check that the
 * system function call convention is returned; all callee-preserved
 * registers are indeed preserved.
 */
#define REGISTERS_DECLARATION   register int a, b, c, d, e, f
#define REGISTERS_PRE           do { a = 0x11111111; \
                                     b = 0x22222222; \
                                     c = 0x33333333; \
                                     d = 0x44444444; \
                                     e = 0x55555555; \
                                     f = 0x66666666; } while (0)
#define REGISTERS_WRONG         (a != 0x11111111 || \
                                 b != 0x22222222 || \
                                 c != 0x33333333 || \
                                 d != 0x44444444 || \
                                 e != 0x55555555 || \
                                 f != 0x66666666)

/**
 * Ensures a signal delivered well before the syscall causes EINTR,
 * and that it can be properly cleared.
 *
 * This doubles as a basic test of sigsafe_nanosleep(), which is remarkable
 * under OS X because it is a Mach system call, different from the others.
 */
int
test_received_flag(void)
{
    struct timespec ts = { .tv_sec = 0, .tv_nsec = 1000 };
    int res;

    res = sigsafe_nanosleep(&ts, NULL);
    if (res != 0) {
        return 1;
    }
    raise(SIGALRM);
    res = sigsafe_nanosleep(&ts, NULL);
    if (res != -EINTR) {
        return 1;
    }
    res = sigsafe_nanosleep(&ts, NULL);
    if (res != -EINTR) {
        return 1;
    }
    sigsafe_clear_received();
    res = sigsafe_nanosleep(&ts, NULL);
    if (res != 0) {
        return 1;
    }

    return 0;
}

/**
 * Tests that sigsafe_pause() works. This is a simple zero-argument system
 * call, except on platforms where it's implemented by calling sigsuspend().
 */
int
test_pause(void)
{
    struct itimerval it = {
        .it_interval = { .tv_sec = 0, .tv_usec = 0 },
        .it_value = { .tv_sec = 0, .tv_usec = 500 }
    };
    int res;
    REGISTERS_DECLARATION;

    error_wrap(setitimer(ITIMER_REAL, &it, NULL),
               "setitimer", ERRNO);
    REGISTERS_PRE;
    res = sigsafe_pause();
    if (REGISTERS_WRONG) {
        sigsafe_clear_received();
        return 1;
    }
    sigsafe_clear_received();
    if (res != -EINTR) {
        return 1;
    }

    return 0;
}

#ifdef _THREAD_SAFE
static void
test_tsd_usr1(int signo, siginfo_t *si, ucontext_t *ctx, intptr_t user_data)
{
    int *subthread_tsd = (int*) user_data;

    if (*subthread_tsd != 26) {
        abort();
    }
    *subthread_tsd = 37;
}

static void
subthread_tsd_destructor(intptr_t tsd)
{
    int *subthread_tsd = (int*) tsd;

    *subthread_tsd = 42;
}

static void*
test_tsd_subthread(void *arg)
{
    int *subthread_tsd = (int*) arg;

    error_wrap(sigsafe_install_tsd((intptr_t) subthread_tsd,
                                   subthread_tsd_destructor),
               "sigsafe_install_tsd", NEGATIVE);

    *subthread_tsd = 26;
    error_wrap(sigsafe_install_handler(SIGUSR1, test_tsd_usr1),
               "sigsafe_install_handler", NEGATIVE);
    raise(SIGUSR1);
    /*
     * Note: never clearing received.
     * This should not affect the main thread.
     */
    if (*subthread_tsd != 37) {
        return (void*) 1;
    }

    return (void*) 0;
}

int
test_tsd(void)
{
    pthread_t subthread;
    int subthread_tsd, res;
    struct timespec ts = { .tv_sec = 0, .tv_nsec = 1 };

    tsd = 0;
    pthread_create(&subthread, NULL, test_tsd_subthread, &subthread_tsd);
    pthread_join(subthread, (void**) &res);
    if (res != 0) {
        return 1;
    }
    if (subthread_tsd != 42 /* set by destructor */) {
        return 1;
    }

    /* Subthread's flag shouldn't be honored here. */
    res = sigsafe_nanosleep(&ts, NULL);
    if (res != 0) {
        return 1;
    }

    return 0;
}
#endif

struct test {
    char *name;
    int (*func)(void);
} tests[] = {
#define DECLARE(name) { #name, name }
    DECLARE(test_received_flag),
    DECLARE(test_pause),
#ifdef _THREAD_SAFE
    DECLARE(test_tsd),
#endif
#undef DECLARE
};

int
main(int argc, char **argv)
{
    int result = 0;
    struct test *t;

    error_wrap(sigsafe_install_handler(SIGALRM, NULL),
               "sigsafe_install_handler", NEGATIVE);

    error_wrap(sigsafe_install_tsd((intptr_t) &tsd, NULL),
               "sigsafe_install_tsd", NEGATIVE);

    for (t = tests; t < tests + sizeof(tests)/sizeof(tests[0]); t++) {
        int this_result;

        printf("%s: ", t->name);
        fflush(stdout);
        this_result = t->func();
        printf("%s\n", (this_result == 0) ? "success" : "FAILURE");
        result |= this_result;
    }

    return (result == 0) ? 0 : 1;
}
