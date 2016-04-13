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

int sys___read(int fd, const void *buf, size_t nbytes, int32_t *retval)
{
    struct proc* test = curproc;
    if (fd < 0 || fd > OPEN_MAX || curproc->ft[fd] == NULL)
    {
        return EBADF;
    }
    if (curproc->ft[fd]->flags == O_WRONLY)
    {
        return EBADF;
    }
    int result;
    
    struct iovec iov;
    struct uio u;
    
    uio_kinit(&iov,&u,(void *)buf,nbytes,curproc->ft[fd]->offset,UIO_READ);
    
    result= VOP_READ(curproc->ft[fd]->path,&u);
    
    //catches EFAULT
    if (result)
    {
        return result;
    }
    
    *retval = nbytes - u.uio_resid;
    struct handler* handle = curproc->ft[fd];
    handle->offset = handle->offset + *retval;
    (void)test;
    return 0;
}