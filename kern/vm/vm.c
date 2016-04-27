#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <spinlock.h>
#include <cpu.h>
#include <current.h>
#include <mips/tlb.h>
#include <spl.h>

void vm_bootstrap(void){
    //Done in ram_bootstrap
}

int vm_fault(int faulttype, vaddr_t faultaddress){
	switch (faulttype){
	    case VM_FAULT_READONLY:
		/* We always create pages read-write, so we can't get this */
		panic("vm: got VM_FAULT_READONLY\n");
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		break;
	    default:
		return EINVAL;
	}
    if(curproc == NULL){
        return EFAULT;
    }
    struct addrspace *as = proc_getas();
    if (as == NULL){
        return EFAULT;
    }

	struct pgtentry* pgt = as->pgt;
    struct region* r = as->region;
    bool foundpgte = false;
    vaddr_t vaddr = faultaddress & PAGE_FRAME;
    paddr_t paddr = 0;
	
	KASSERT(pgt != NULL);
    KASSERT(r != NULL);
	
	int spl;
	uint32_t ehi, elo;
	
	bool valid = false;
	while(r->next != NULL){
        r = r->next;
		if(vaddr >= r->start && vaddr < r->start+r->pages*PAGE_SIZE){
			valid = true;
			break;
		}
	}
	if(!valid){
		return EFAULT;
	}
	
    while(pgt->next != NULL){
        pgt = pgt->next;
        if(pgt->vpn == vaddr){
            foundpgte = true;
			paddr = pgt->ppn;
			break;
        }
    }
	
    if(!foundpgte){
        while(pgt->next != NULL)
            pgt = pgt->next;{
        }
		
        pgt->next = kmalloc(sizeof(struct pgtentry));
		if(pgt->next == NULL){
			return ENOMEM;
		}
        pgt->next->vpn = vaddr;
        pgt->next->ppn = KVADDR_TO_PADDR(alloc_kpages(1));
        pgt->next->permission = faulttype;
        pgt->next->state = false;
        pgt->next->next = NULL;
		paddr = pgt->next->ppn;
    }
	
	spl = splhigh();
    int index = tlb_probe(faultaddress,0);
	if(index < 0){
        for (int i=0; i<NUM_TLB; i++) {
            tlb_read(&ehi, &elo, i);
            if (elo & TLBLO_VALID) {
                continue;
            }
            ehi = vaddr;
            elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
            tlb_write(ehi, elo, i);
            splx(spl);
            return 0;
        }
    }
    else{
        tlb_read(&ehi, &elo, index);
        ehi = vaddr;
		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		tlb_write(ehi, elo, index);
		splx(spl);
		return 0;
    }
	splx(spl);
	return EFAULT;
}

vaddr_t alloc_kpages(unsigned npages){
    int index = 0;
    spinlock_acquire(&coremap_lock);
	for(int i = fixed_pages;i < max_pages;i++){
        if(coremap[i].state == free){
            int freepages = 1;
            for(int j = 1;j < (int)npages;j++){
                if(coremap[i+j].state != free){
                    break;
                }
                freepages++;
            }
            if(freepages == (int)npages){
                for(int j = 1;j < (int)npages;j++){
                    coremap[i+j].state = dirty;
                    coremap[i+j].nsize = 0;
                }
                coremap[i].state = dirty;
                coremap[i].nsize = npages*PAGE_SIZE;
                coremap[i].vaddr = PADDR_TO_KVADDR(i*PAGE_SIZE);
                coremap_bytes += (npages*PAGE_SIZE);
                index = i;
                break;
            }
        }
    }
    // kprintf("%d bytes alloc\n",coremap_bytes);
    spinlock_release(&coremap_lock);
    if(index == 0){
        return 0;
    }
	
	return coremap[index].vaddr;
}

void free_kpages(vaddr_t vaddr){
	paddr_t paddr = KVADDR_TO_PADDR(vaddr);
	int index = paddr/PAGE_SIZE;
	
	spinlock_acquire(&coremap_lock);
	if(coremap[index].vaddr == vaddr){
		if(coremap[index].state != free && coremap[index].nsize != 0){
			int pages = coremap[index].nsize/PAGE_SIZE;
			for(int i = 0;i < pages;i++){
				coremap[index+i].state = free;
			}
			coremap_bytes -= coremap[index].nsize;
			// kprintf("%d bytes free\n",coremap_bytes);
			coremap[index].nsize = 0;
		}
		else{
			coremap[index].state = free;
		}
	}
	spinlock_release(&coremap_lock);
}

__size_t coremap_used_bytes(void){
	return coremap_bytes;
}

void vm_tlbshootdown_all(void){
	
}

void vm_tlbshootdown(const struct tlbshootdown *ts){
	(void)ts;
}