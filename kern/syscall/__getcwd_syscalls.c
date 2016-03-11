#include <types.h>
#include <vfs.h>
#include <syscall.h>
#include <uio.h>
#include <kern/errno.h>

int sys___getcwd(userptr_t buf,size_t buflen,int32_t *retval){
    int result;
    
        //error handling
    if(buf == NULL)
    {
        return EFAULT;
    }
    
    struct iovec *iov = kmalloc(sizeof(*iov));
    struct uio *u = kmalloc(sizeof(*u));
    uio_kinit(iov,u,(void *)buf,buflen,0,UIO_READ);
    
    result = vfs_getcwd(u);
    if(result){
        return result;
    }
    
    *retval = buflen - u->uio_resid;
    
    kfree(iov);
    kfree(u);
    
    return 0;
}