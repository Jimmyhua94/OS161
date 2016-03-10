

off_t sys___lseek(int fd, off_t pos, int whence, int32_t *retval){
    if(whence != SEEK_SET || whence != SEEK_CUR || whence != SEEK_END){
        return EINVAL;
    }
    
    if(fd < 3){
        return ESPIPE;
    }
    
    if(curproc->ft[fd] == NULL){
        return EBADF;
    }
    
    if(whence == SEEK_SET){
        if(pos > 0){
            curproc->ft[fd]->offset = pos;
        }
        else{
            return EINVAL;
        }
    }
    else if(whence == SEEK_CUR){
        if(curproc->ft[fd]->offset + pos < 0){
            curproc->ft[fd]->offset += pos;
        }
        else{
            return EINVAL;
        }
    }
    else if(whence == SEEK_END){
        if(end + pos < 0){
            curproc->ft[fd]->offset = end+pos;
        }
        else{
            return EINVAL;
        }
    }
    
    *retval = curproc->ft[fd]->offset;
    
    return 0;
}