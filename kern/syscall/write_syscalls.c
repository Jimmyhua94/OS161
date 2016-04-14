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

int sys___write(int fd, userptr_t buf, size_t nbytes, int32_t *retval){
    if(fd < 0 || fd > OPEN_MAX || curproc->ft[fd] == NULL){
        return EBADF;
    }
    if(curproc->ft[fd]->flags & O_RDONLY){
        return EBADF;
    }
    
    int result; 
    
    struct iovec iov;
    struct uio u;
    lock_acquire(curproc->ft[fd]->lock);
    uio_kinit(&iov,&u,buf,nbytes,curproc->ft[fd]->offset,UIO_WRITE);
	
    result = VOP_WRITE(curproc->ft[fd]->path,&u);
    if (result){
        lock_release(curproc->ft[fd]->lock);
        return result;
    }
    int offset = nbytes - u.uio_resid;
	if(VOP_ISSEEKABLE(curproc->ft[fd]->path)){
		curproc->ft[fd]->offset += offset;
	}
    lock_release(curproc->ft[fd]->lock);
    *retval = offset;
    return 0;
}