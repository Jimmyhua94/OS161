#include <types.h>
#include <vnode.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <kern/errno.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <lib.h>

int sys___lseek(int fd, off_t pos, int whence, int64_t *retval){
    int64_t result;
    if(fd < 3 || fd > OPEN_MAX || curproc->ft[fd] == NULL){
        return EBADF;
    }
    
    if(!VOP_ISSEEKABLE(curproc->ft[fd]->path)){
        return ESPIPE;
    }
    lock_acquire(curproc->ft[fd]->lock);
    if(whence == SEEK_SET){
        if(pos >= 0){
            curproc->ft[fd]->offset = pos;
        }
        else{
            lock_release(curproc->ft[fd]->lock);
            return EINVAL;
        }
    }
    else if(whence == SEEK_CUR){
        if(curproc->ft[fd]->offset + pos >= 0){
            curproc->ft[fd]->offset += pos;
        }
        else{
            lock_release(curproc->ft[fd]->lock);
            return EINVAL;
        }
    }
    else if(whence == SEEK_END){
        struct stat stats;
        result = VOP_STAT(curproc->ft[fd]->path,&stats);
        if (result){
            lock_release(curproc->ft[fd]->lock);
            return result;
        }
        off_t end = stats.st_size;
        if(end + pos > 0){
            curproc->ft[fd]->offset = end+pos;
        }
        else{
            lock_release(curproc->ft[fd]->lock);
            return EINVAL;
        }
    }
    else{
        lock_release(curproc->ft[fd]->lock);
        return EINVAL;
    }
    *retval = curproc->ft[fd]->offset;
    lock_release(curproc->ft[fd]->lock);
    
    return 0;
}