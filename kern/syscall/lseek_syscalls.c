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
    if(whence == SEEK_SET){
        if(pos >= 0){
            curproc->ft[fd]->offset = pos;
        }
        else{
            return EINVAL;
        }
    }
    else if(whence == SEEK_CUR){
        if(curproc->ft[fd]->offset + pos >= 0){
            curproc->ft[fd]->offset += pos;
        }
        else{
            return EINVAL;
        }
    }
    else if(whence == SEEK_END){
        struct stat *stats = kmalloc(sizeof(*stats));
        result = VOP_STAT(curproc->ft[fd]->path,stats);
        if (result){
            return result;
        }
        off_t end = stats->st_size;
        kfree(stats);
        if(end + pos > 0){
            curproc->ft[fd]->offset = end+pos;
        }
        else{
            return EINVAL;
        }
    }
    else{
        return EINVAL;
    }
    *retval = curproc->ft[fd]->offset;
    
    return 0;
}