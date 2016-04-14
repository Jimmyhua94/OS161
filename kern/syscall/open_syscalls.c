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
#include <kern/stat.h>

int sys___open(const_userptr_t filename, int flags,mode_t mode,int32_t *retval){
    int result;
    
    char filepath[PATH_MAX];
    size_t size = 0;
    
    result = copyinstr(filename,filepath, PATH_MAX,&size);
    
    //handles ENODEV, ENOTDIR, ENOENT, ENOENT, (maybe?)EEXIST, EFAULT 
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
        kfree(handle);
        return result;
    }
    
	lock_acquire(curproc->fdlock);
    for (handle->fd = 0; handle->fd < OPEN_MAX; handle->fd++){
        if(curproc->ft[handle->fd] == NULL){
            curproc->ft[handle->fd] = handle;
            handle->lock = lock_create(handle->fd + "_lock");
            break;
        }
    }
	lock_release(curproc->fdlock);
    lock_acquire(handle->lock);
    handle->path = v;
    if (flags & O_APPEND){
        struct stat *stats = kmalloc(sizeof(*stats));
        result = VOP_STAT(v,stats);
        if (result){
            lock_release(handle->lock);
            return result;
        }
        handle->offset = stats->st_size;
        kfree(stats);
    }
    else{
        handle->offset = 0;
    }
    if (flags & O_RDONLY){
        handle->flags |= O_RDONLY;
    }
    else if (flags & O_WRONLY){
        handle->flags |= O_WRONLY;
    }
    else if (flags & O_RDWR){
        handle->flags |= O_RDWR;
    }
    handle->count++;
    
    *retval = handle->fd;
    lock_release(handle->lock);
    
    return 0;
}