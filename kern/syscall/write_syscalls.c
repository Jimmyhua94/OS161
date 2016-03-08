#include <types.h>
#include <vfs.h>
#include <vnode.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <copyinout.h>
#include <limits.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <uio.h>


int sys___write(int fd, const void *buf, size_t nbytes){
    if(curthread->ft[fd] == NULL){
        return -1;
    }
    int result;
    struct iovec *iov = kmalloc(sizeof(*iov));
    struct uio *u = kmalloc(sizeof(*u));
    uio_kinit(iov,u,(void *)buf,nbytes,curthread->ft[fd]->offset,UIO_WRITE);
    
    result = VOP_WRITE(curthread->ft[fd]->path,u);
    if (result){
        return -1;
    }
    kfree(iov);
    kfree(u);
    return 0;
}