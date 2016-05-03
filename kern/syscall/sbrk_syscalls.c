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
	amount &= PAGE_FRAME;
	vaddr_t temp = as->heap_end + amount;
	if(temp < as->heap_start){
        return EINVAL;
    }
    vaddr_t stackbase = (USERSTACK - 1024 * PAGE_SIZE);
    if(temp > stackbase){
        return ENOMEM;
    }
    if(amount < 0){
        int spl;
        int npages = (amount/PAGE_SIZE)*-1;
        vaddr_t rempage = temp;
        for(int i = 0;i < npages;i++){
            struct pgtentry* pgt = as->pgt;
            struct pgtentry* last = pgt;
            // lock_acquire(curproc->lock);
            while(pgt->next!=NULL){
                last = pgt;
                pgt = pgt->next;
                if(pgt->vpn == rempage){
                    spl = splhigh();
                    int index = tlb_probe(rempage,0);
                    if(index >= 0)
                        tlb_write(TLBHI_INVALID(index),TLBLO_INVALID(),index);
                    splx(spl);
                    free_kpages(PADDR_TO_KVADDR(pgt->ppn));
                    last->next = pgt->next;
                    kfree(pgt);
                    break;
                }
            }
            // lock_release(curproc->lock);
            rempage += PAGE_SIZE;
        }
    }
    *retval = as->heap_end;

    as->heap_end = temp;

	return 0;
}