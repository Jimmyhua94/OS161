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

int sys___open(const_userptr_t filename, int flags,mode_t mode,int32_t *retval){
    int result;
    if (!((flags == O_RDONLY) | (flags == O_WRONLY) | (flags == O_RDWR))){
        return EINVAL;
    }
    
    char filepath[__PATH_MAX];
    size_t *size = 0;
    
    result = copyinstr(filename,filepath,__PATH_MAX,size);
    
    if (result){
        return result;
    }

    struct vnode *v;
    //struct handler handler;
    struct handler* handle = kmalloc(sizeof(*handle));
    //handle = &handler;
    
    if (!mode){
        result = vfs_open(filepath,flags,0,&v);
    }
    else{
        result = vfs_open(filepath,flags,mode,&v);
    }
    
    if (result){
        return result;
    }
    
    for (handle->fd = 0; handle->fd < __OPEN_MAX; handle->fd++){
        if(curthread->ft[handle->fd] == NULL){
            break;
        }
    }
    handle->path = v;
    handle->offset = 0;
    handle->flags = flags;
    handle->count = 1;
    
    curthread->ft[handle->fd] = handle;
    *retval = handle->fd;
    return 0;
}