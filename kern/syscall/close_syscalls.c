#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <limits.h>
#include <kern/errno.h>
#include <vfs.h>
#include <synch.h>

int sys___close(int fd){
    if(fd < 0 || fd > OPEN_MAX || curproc->ft[fd] == NULL){
        return EBADF;
    }
	lock_acquire(curproc->ft[fd]->lock);
	curproc->ft[fd]->count--;
    if(curproc->ft[fd]->count == 0){
		vfs_close(curproc->ft[fd]->path);
		lock_release(curproc->ft[fd]->lock);
        kfree(curproc->ft[fd]);
        curproc->ft[fd] = NULL;
    }
    return 0;
}