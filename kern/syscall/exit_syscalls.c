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
    
    //KASSERT
    // as_deactivate();
    
    // as = curproc_setas(NULL);
    // as_destroy(as);
    
    // proc_remthread(curthread);
    
    // proc_destroy(p);
    
    // thread_exit();
}