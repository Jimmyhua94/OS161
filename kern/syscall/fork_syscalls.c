#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <addrspace.h>
#include <syscall.h>


int sys__fork(struct trapframe *tf,int *retval)
{
    struct trapframe *tf_child = kmalloc(sizeof(struct trapframe));
    if (tf_child ==NULL)
    {
        return result;
    }
    
    struct addrspace * child_addrspace;
    int result = as_copy(curthread->t_addrspace, &child_addrspace);
    if(result)
    {
        return result;
    }
    
    result = thread_fork((struct trapframe *)tf_child,(unsigned long) child_addrspace, &ch_thread);
    
    *retval = ch_thread->pid;
    
    if (result)
    {
        return result;
    }
    
    return 0;
}