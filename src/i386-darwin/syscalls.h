/** @file
 * Lists implemented raw system calls on ppc-darwin.
 * @legal
 * Copyright &copy 2004 Scott Lamb &lt;slamb@slamb.org&gt;.
 * This file is part of sigsafe, which is released under the MIT license.
 * @version     $Id$
 * @author      Scott Lamb &lt;slamb@slamb.org&gt;
 */

SYSCALL(accept, 3)
SYSCALL(connect, 3)
#define SYS_clock_sleep_trap -62 /* from xnu/osfmk/mach/syscall_sw.h */
MACH_SYSCALL(clock_sleep_trap, 5)
#ifdef SIGSAFE_HAVE_KEVENT
SYSCALL(kevent, 6)
#endif
SYSCALL(open, 3)
SYSCALL(read, 3)
SYSCALL(readv, 3)
/* recv is emulated */
SYSCALL(recvfrom, 6)
SYSCALL(recvmsg, 3)
/* send is emulated */
SYSCALL(sendto, 6)
SYSCALL(sendmsg, 3)
#define SYS_sigsuspend_ SYS_sigsuspend
SYSCALL(sigsuspend_, 1)
SYSCALL(write, 3)
SYSCALL(writev, 3)
SYSCALL(select, 5)
SYSCALL(wait4, 4)
