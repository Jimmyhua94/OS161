#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>

void sys__exit(int exitcode)
{
    struct addrspace *as;
    
	curproc->exited = 1;
    curproc->exitcode = _MKWAIT_EXIT(exitcode);
    
    thread_exit();
    
    V(curproc->waitsem);
    
    struct proc* = getproc(pidIndex);
    
    if(proc->p_numthreads == 0){
        exorcise();
        proc_destroy(curproc);
    }
}