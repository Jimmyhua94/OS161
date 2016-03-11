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
    
    char filepath[PATH_MAX];
    size_t size = 0;
    
    char *argoff = kmalloc(ARG_MAX * sizeof(userptr_t)); //offset of arguments
    char *kargs = kmalloc(sizeof(ARG_MAX));  //char arguments
    
    char *offset = kargs;
    
    result = copyinstr(program,filepath, PATH_MAX,&size);
    if (result){
        return result;
    }
    
    //copy args in
    userptr_t ptr;
    size_t argsize = 0;
    int argoffcount = 0;
    
    int i = 0;
    do{
        result = copyin(args,&ptr,sizeof(userptr_t));
        if(result){
            return result;
        }
        if(ptr != NULL){
            result = copyinstr(ptr,offset,ARG_MAX,&size);
            if(result){
                return result;
            }
            argsize += size;
            if((size + 1)%4 == 0){
                offset += size+1;
                argoff[i] = argoffcount;
                argoffcount += size+1;
            }
            else if((size + 2)%4 == 0){
                offset += size+2;
                argoff[i] = argoffcount;
                argoffcount += size+2;
            }
            else if((size + 3)%4 == 0){
                offset += size+3;
                argoff[i] = argoffcount;
                argoffcount += size+3;
            }
            else{
                offset += size;
                argoff[i] = argoffcount;
                argoffcount += size;
            }
            args+=4;
        }
        i++;
    }while(ptr != NULL && argsize > ARG_MAX);
    if(argsize > ARG_MAX){
        return E2BIG;
    }

    struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;

	/* Open the file. */
	result = vfs_open(filepath, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

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
    
    int finaloff = i*4;
    
    for(int j = 0; j < i-1;j++){
        argoff[i]+= *(&stackptr+finaloff);
    }
    userptr_t ptrstack = (userptr_t) stackptr;
    result = copyout(argoff,ptrstack,4*i);
    if (result){
        return result;
    }
    ptrstack = (userptr_t)stackptr+(4*i);
    result = copyout(kargs,ptrstack,ARG_MAX);
    
    /* Warp to user mode. */
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
    
}