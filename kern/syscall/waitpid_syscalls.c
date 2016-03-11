

int sys___waitpid(pid_t pid, int *status, int options, int32_t *retval){
    void(options)
    int pidIndex = getpid(pid);
    if(pidIndex != -1){
        if(exited(pidIndex)){
            return exitcode(pidIndex);
        }
        //sleep and wait
        *status = exitcode(pidIndex);
        *retval = pid;
        
        if(curproc->p_numthreads == 0){
            exorcise();
        }
        return(0);
    }
    return ESRCH;
}