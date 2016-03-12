#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <addrspace.h>
#include <syscall.h>
#include <kern/errno.h>
#include <mips/trapframe.h>
#include <spl.h>

void child_forkentry(void *tf, unsigned long addrspace){
    struct trapframe child_tf = *(struct trapframe*)tf;
    child_tf.tf_v0 = 0;
    child_tf.tf_a3 = 0;
    child_tf.tf_epc += 4;
    curproc->p_addrspace = (struct addrspace*)addrspace;
    as_activate();
    mips_usermode(&child_tf);
}

int sys___fork(struct trapframe *tf,int *retval)
{
    int spl;
    struct trapframe *tf_child = kmalloc(sizeof(struct trapframe));
    if (tf_child == NULL)
    {
        return ENOMEM;
    }
    memcpy(tf_child,tf,sizeof(struct trapframe));
    
    struct proc* child_proc = proc_create_runprogram(curthread->t_name);
    
    struct addrspace* child_addrspace;
    int result = as_copy(curproc->p_addrspace, &child_addrspace);
    if(result)
    {
        return result;
    }
    
    spl = splhigh();
    result = thread_fork(curthread->t_name, child_proc, child_forkentry, tf_child, (unsigned long)child_addrspace);
    splx(spl);
    if (result)
    {
        return result;
    }
    kfree(tf_child);
    *retval = child_proc->pid;
    return 0;
}