#include <types.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <synch.h>

int sys___exit(int exitcode)
{
	curproc->exited = 1;
    curproc->exitcode = _MKWAIT_EXIT(exitcode);
    
    thread_exit();
    
	if(curproc->lock != NULL)
		cv_broadcast(curproc->waitlock,curproc->lock);
    else
		proc_destroy(curproc);
    return 0;
}