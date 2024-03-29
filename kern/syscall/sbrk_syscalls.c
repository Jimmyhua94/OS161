#include <types.h>
#include <syscall.h>
#include <current.h>
#include <thread.h>
#include <proc.h>
#include <kern/errno.h>
#include <addrspace.h>
#include <spl.h>
#include <mips/tlb.h>


int sys___sbrk(int amount, int32_t *retval){
	struct addrspace *as = proc_getas();
	if(amount%4 != 0){
        *retval = -1;
        return EINVAL;
    }
    if(amount < 0){
        int amt = amount*-1;
        int size = as->heap_end-as->heap_start;
        if(amt > size){
            *retval = -1;
            return EINVAL;
        }
    }
    vaddr_t temp = as->heap_end + amount;
    vaddr_t stackbase = (USERSTACK - 1024 * PAGE_SIZE);
    if(temp > stackbase){
        *retval = -1;
        return ENOMEM;
    }
    if(amount < 0){
        int spl;
		struct pgtentry* pgt = as->pgt;
		struct pgtentry* last = pgt;
		// lock_acquire(coremap_biglock);
		while(pgt->next!=NULL){
			last = pgt;
			pgt = pgt->next;
			if(pgt->heap && (pgt->vpn >= temp && pgt->vpn < as->heap_end)){
				spl = splhigh();
				int index = tlb_probe(pgt->vpn,0);
				if(index >= 0)
					tlb_write(TLBHI_INVALID(index),TLBLO_INVALID(),index);
				splx(spl);
				free_kpages(PADDR_TO_KVADDR(pgt->ppn));
				last->next = pgt->next;
				kfree(pgt);
				break;
			}
		}
		// lock_release(coremap_biglock);
    }
    *retval = as->heap_end;

    as->heap_end = temp;

	return 0;
}