#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <limits.h>
#include <kern/errno.h>

int sys___close(int fd){
    if(fd < 0 || fd > OPEN_MAX || curproc->ft[fd] == NULL){
        return EBADF;
    }
	curproc->ft[fd]->count--;
    if(curproc->ft[fd]->count == 0){
        kfree(curproc->ft[fd]);
        curproc->ft[fd] = NULL;
    }
    return 0;
}