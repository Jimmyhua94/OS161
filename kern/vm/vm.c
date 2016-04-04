#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <spinlock.h>
#include <cpu.h>
#include <current.h>

void vm_bootstrap(void){
}

int vm_fault(int faulttype, vaddr_t faultaddress){
	(void)faulttype;
	(void)faultaddress;
	return 0;
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