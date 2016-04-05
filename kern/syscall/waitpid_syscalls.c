#include <types.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <synch.h>
#include <kern/errno.h>


int sys___waitpid(pid_t pid, userptr_t status, int options, int32_t *retval){
    (void)options;
    if(pid == curproc->pid){
        return ECHILD;
    }
    
    int pidIndex = getpidIndex(pid);
    if(pidIndex != -1){
        //getproc(pidIndex)->waitsem = sem_create("waitsem",0);
		getproc(pidIndex)->waitlock = cv_create("waitlock");
		getproc(pidIndex)->lock = lock_create("waitlocklock");
        if(getppid(pidIndex) != curproc->pid){
            return ECHILD;
        }
        if(!exited(pidIndex)){
			lock_acquire(getproc(pidIndex)->lock);
			cv_wait(getproc(pidIndex)->waitlock,getproc(pidIndex)->lock);
			lock_release(getproc(pidIndex)->lock);
			//P(getproc(pidIndex)->waitsem);
			*(int*)status = exitcode(pidIndex);
			*retval = pid;
			proc_destroy(getproc(pidIndex));
        }
        return 0;
    }
    return ESRCH;
}