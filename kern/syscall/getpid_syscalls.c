#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>

int sys___gitpid(int32_t *retval){
    *retval = curproc.pid;
    return 0;
}