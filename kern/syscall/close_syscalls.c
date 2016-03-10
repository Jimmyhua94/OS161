#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <limits.h>
#include <kern/errno.h>

int sys___close(int fd){
    if(curproc->ft[fd] == NULL){
        return EBADF;
    }
    kfree(curproc->ft[fd]);
    curproc->ft[fd] = NULL;
    return 0;
}