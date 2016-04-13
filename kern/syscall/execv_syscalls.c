#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include <kern/unistd.h>
#include <copyinout.h>

int sys___execv(const_userptr_t program, userptr_t args){
    int result;
    (void)args;
    
    char filepath[PATH_MAX];
    size_t size = 0;
    
    char **tempargs = (char**)args;
    int maxargs = 0;
    while(tempargs[maxargs] != NULL){
        maxargs++;
    }
    
    int ptr[maxargs+1];
    char *arg = kmalloc(ARG_MAX);
    char *argoff = arg;
    size_t argsize = 0;
    
    //Copy in pathname
    result = copyinstr(program,filepath, PATH_MAX,&size);
    if (result){
        return result;
    }
    
    int i = 0;
    ptr[i] = (maxargs*4)+4;
    while(tempargs[i] != NULL){
        result = copyinstr((userptr_t)tempargs[i],argoff,ARG_MAX,&size);
        if(result){
            return result;
        }
        argoff += size;
        int pad = 0;
        if((size + 1)%4 == 0){
            pad = 1;
        }
        else if((size + 2)%4 == 0){
            pad = 2;
        }
        else if((size + 3)%4 == 0){
            pad = 3;
        }
        for(int j = 0;j < pad;j++){
            *argoff = '\0';
            argoff++;
        }
        int offsize = size+pad;
        argsize += offsize;
        ptr[i+1] = ptr[i]+offsize;
        if(argsize > ARG_MAX){
            return E2BIG;
        }
        i++;
    }

    struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;

	/* Open the file. */
	result = vfs_open(filepath, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}
	
	as_destroy(curproc->p_addrspace);
	curproc->p_addrspace = NULL;

	/* We should be a new process. */
	KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}
	
	curproc->p_name = tempargs[0];
    
	//assign address to each offset
    userptr_t ptrs[maxargs+1];
    userptr_t ptrstack = (userptr_t)stackptr-sizeof(ptrs)-argsize;
    stackptr = (vaddr_t)ptrstack;
    for(int j = 0;j < maxargs;j++){
        ptrs[j] = (userptr_t)(ptr[j]+ptrstack);
    }
    ptrs[maxargs+1] = NULL;
    result = copyout(&ptrs,ptrstack,sizeof(ptrs));
    if(result){
        return result;
    }
	
    ptrstack += sizeof(ptrs);
    result = copyout(arg,ptrstack,argsize);
    if(result){
        return result;
    }
    
    /* Warp to user mode. */
	enter_new_process(maxargs, (userptr_t)stackptr,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
    
}