#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>

#define VM_STACKPAGES	64

static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

void vm_bootstrap(void){
	coremap_bytes = 0;
}

int vm_fault(int faulttype, vaddr_t faultaddress){
	
}

vaddr_t alloc_kpages(unsigned npages){
	paddr_t pa;
	
	pa = getppages(npages);
	if(pa == 0){
		return 0;
	}
	
	coremap_bytes += (npages*PAGE_SIZE);
	
	return PADDR_TO_KVADDR(pa);s
}

void free_kpages(vaddr_t addr){
	
}

unsigned int coremap_used_bytes(void){
	return coremap_bytes;
}

void vm_tlbshootdown_all(void){
	
}

void vm_tlbshootdown(cons struct tlbshootdown *){
	
}