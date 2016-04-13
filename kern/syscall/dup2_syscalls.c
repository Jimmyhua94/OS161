#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <limits.h>
#include <kern/errno.h>


int sys___dup2(int oldfd, int newfd, int32_t *retval){
    if(oldfd < 0 || newfd < 0 || newfd > OPEN_MAX || oldfd > OPEN_MAX || curproc->ft[oldfd] == NULL){
        return EBADF;
    }
    
    if(oldfd == newfd){
        return 0; 
    }
    
    curproc->ft[newfd] = curproc->ft[oldfd];
    
    *retval = newfd;
    
    return 0;
}