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
#include <copyinout.h>

int sys___waitpid(pid_t pid, userptr_t status, int options, int32_t *retval){
    (void)options;
    if(pid == curproc->pid){
        return ECHILD;
    }
    
    int pidIndex = getpidIndex(pid);
    if(pidIndex != -1){
        struct proc* child_proc = getproc(pidIndex);
		child_proc->waitlock = cv_create("waitlock");
		child_proc->lock = lock_create("waitlocklock");
        if(child_proc->ppid != curproc->pid){
            return ECHILD;
        }
        // if(!(child_proc->exited)){
			lock_acquire(child_proc->lock);
			child_proc->waiting = true;
			while(!(child_proc->exited)){
				cv_wait(child_proc->waitlock,child_proc->lock);
			}
			lock_release(child_proc->lock);
            int exitcode = child_proc->exitcode;
            int result = copyout(&exitcode,status,sizeof(int));
            if (result){
                return result;
            }
			*retval = pid;
        // }
        return 0;
    }
    return ESRCH;
}