#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <kern/errno.h>
#include <addrspace.h>

int sys___sbrk(int amount, int32_t *retval){
	struct addrspace *as = proc_getas();
	amount = ROUNDUP(amount,4);
	paddr_t temp = as->heap_end + amount;
	KASSERT(temp >= as->heap_start);
	as->heap_end = temp;

	*retval = as->heap_end;
	return 0;
}