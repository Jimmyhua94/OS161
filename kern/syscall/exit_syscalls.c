#include <types.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <synch.h>

void sys___exit(int exitcode)
{
	curproc->exited = 1;
    curproc->exitcode = _MKWAIT_EXIT(exitcode);
    
	if(curproc->waiting){
		lock_acquire(curproc->lock);
		cv_broadcast(curproc->waitlock,curproc->lock);
		lock_release(curproc->lock);
	}
    thread_exit();
}