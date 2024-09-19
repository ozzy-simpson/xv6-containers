#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(prio_set)
SYSCALL(cm_create_and_enter)
SYSCALL(cm_setroot)
SYSCALL(cm_maxproc)
SYSCALL(shm_get)
SYSCALL(shm_rem)
SYSCALL(shm_print)
SYSCALL(shm_retaddr)
SYSCALL(mut_create)
SYSCALL(mut_delete)
SYSCALL(mut_lock)
SYSCALL(mut_unlock)
SYSCALL(c_wait)
SYSCALL(c_signal)
