<<<<<<< HEAD
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
    struct proc *p;
    p = curproc;
    
    (void)exitcode;
    
    //KASSERT
    as_deactivate();
    
    as = curproc_setas(NULL);
    as_destroy(as);
    
    proc_remthread(curthread);
    
    proc_destroy(p);
    
    thread_exit();
=======


void sys___exit(int exitcode){
    
>>>>>>> 443da2642d8ab9448b0d996bbdf8c538505ceb12
}