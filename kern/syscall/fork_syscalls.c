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
#include <synch.h>
#include <vnode.h>


static
struct proc *
proc_create(const char *name)
{
	struct proc *proc;

	proc = kmalloc(sizeof(*proc));
	if (proc == NULL) {
		return NULL;
	}
	proc->p_name = kstrdup(name);
	if (proc->p_name == NULL) {
		kfree(proc);
		return NULL;
	}

	proc->p_numthreads = 0;
	spinlock_init(&proc->p_lock);

	/* VM fields */
	proc->p_addrspace = NULL;

	/* VFS fields */
	proc->p_cwd = NULL;
	
	spinlock_acquire(&pid_lock);
	proc->pid = pidCounter++;
	for(proc->procIndex = 0;proc->procIndex < PID_MAX; proc->procIndex++){
		if(pt[proc->procIndex] == NULL){
			break;
		}
	}
	pt[proc->procIndex] = proc;
	spinlock_release(&pid_lock);
    
    proc->ppid = -1;
    proc->exited = false;
    proc->exitcode = 0;
    
    
    proc->fdlock = curproc->fdlock;
    proc->lock = lock_create("waitlock");
    
    //memset(proc->ft,0,sizeof(proc->ft));

	return proc;
}

void child_forkentry(void *tf, unsigned long addrspace){
	// curproc->p_addrspace = (struct addrspace*)addrspace;
	(void)addrspace;
    struct trapframe child_tf = *(struct trapframe*)tf;
    child_tf.tf_v0 = 0;
    child_tf.tf_a3 = 0;
    child_tf.tf_epc += 4;
    mips_usermode(&child_tf);
}

int sys___fork(struct trapframe *tf,int *retval)
{
    //int spl;
    struct trapframe *tf_child = kmalloc(sizeof(struct trapframe));
    if (tf_child == NULL)
    {
        return ENOMEM;
    }
    memcpy(tf_child,tf,sizeof(struct trapframe));
    
    struct proc* child_proc = proc_create("test");
    child_proc->ppid = curproc->pid;
	child_proc->p_cwd = curproc->p_cwd;
	VOP_INCREF(child_proc->p_cwd);
    lock_acquire(curproc->fdlock);
	for(int i = 0;i < OPEN_MAX;i++){
		if(curproc->ft[i]!= NULL){
			curproc->ft[i]->count++;
		}
		child_proc->ft[i] = curproc->ft[i];
	}
    lock_release(curproc->fdlock);
    struct addrspace* child_addrspace;
    int result = as_copy(curproc->p_addrspace, &child_addrspace);
    if(result)
    {
        return result;
    }
	child_proc->p_addrspace = child_addrspace;
    
    // spl = splhigh();
    result = thread_fork("test", child_proc, child_forkentry, tf_child, 0);
    // splx(spl);
    if (result)
    {
        return result;
    }
    //kfree(tf_child);
    *retval = child_proc->pid;
    return 0;
}