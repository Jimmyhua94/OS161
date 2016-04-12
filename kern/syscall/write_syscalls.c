#include <types.h>
#include <vfs.h>
#include <vnode.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <copyinout.h>
#include <limits.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <uio.h>
#include <synch.h>


int sys___write(int fd, const void *buf, size_t nbytes, int32_t *retval){
    if(fd < 0 || fd > OPEN_MAX || curproc->ft[fd] == NULL){
        return EBADF;
    }
    if(curproc->ft[fd]->flags & O_RDONLY){
        return EBADF;
    }
    
    int result; 
    
    struct iovec iov;
    struct uio u;
    lock_acquire(curproc->fdlock);
    uio_kinit(&iov,&u,(void *)buf,nbytes,curproc->ft[fd]->offset,UIO_WRITE);
	
    result = VOP_WRITE(curproc->ft[fd]->path,&u);
    if (result){
        return result;
    }
    
    curproc->ft[fd]->offset = u.uio_offset;
    *retval = nbytes - u.uio_resid;
    lock_release(curproc->fdlock);

    return 0;
}