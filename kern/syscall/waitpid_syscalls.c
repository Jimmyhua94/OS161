

int sys___waitpid(pid_t pid, int *status, int options, int32_t *retval){
    void(options)
    if(pid == curproc->pid){
        return ECHILD;
    }
    
    curproc->waitsem = sem_create("waitsem_0");
    
    int pidIndex = getpidIndex(pid);
    if(pidIndex != -1){
        if(getppid(pidIndex) != curproc->pid){
            return ECHILD;
        }
        if(exited(pidIndex)){
            return exitcode(pidIndex);
        }
        P(curproc->waitsem);
        *status = exitcode(pidIndex);
        *retval = pid;

        return(0);
    }
    return ESRCH;
}