#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <spinlock.h>
#include <cpu.h>
#include <current.h>

#define VM_STACKPAGES	64

static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

bool bootStrapped = false;

int max_pages;
int fixed_pages;

size_t coremap_bytes;
struct coremap_entry* coremap;

// struct lock* cm_lock;

void vm_bootstrap(void){
	coremap_bytes = 0;
    max_pages = lastpaddr / PAGE_SIZE;  //Should get rounded down because of int
    coremap = (struct coremap_entry*)PADDR_TO_KVADDR(firstpaddr);
    fixed_pages = firstpaddr;
    firstpaddr += max_pages * sizeof(struct coremap_entry);    //Gets space for coremap
    fixed_pages /= PAGE_SIZE;
    for(int i = 0;i < fixed_pages;i++){
        coremap[i].state = fixed;
    }
    for(int i = fixed_pages;i < max_pages;i++){
        coremap[i].state = free;
    }
    bootStrapped = true;
}

int vm_fault(int faulttype, vaddr_t faultaddress){
	(void)faulttype;
	(void)faultaddress;
	return 0;
}

static
void
vm_can_sleep(void)
{
	if (CURCPU_EXISTS()) {
		/* must not hold spinlocks */
		KASSERT(curcpu->c_spinlocks == 0);

		/* must not be in an interrupt handler */
		KASSERT(curthread->t_in_interrupt == 0);
	}
}

vaddr_t alloc_kpages(unsigned npages){
    if(!bootStrapped){
        paddr_t paddr;
        vm_can_sleep();
        spinlock_acquire(&stealmem_lock);
        paddr = ram_stealmem(npages);
        spinlock_release(&stealmem_lock);
        if (paddr==0) {
            return 0;
        }
        return PADDR_TO_KVADDR(paddr);
    }
    int index = 0;
	for(int i = fixed_pages;i < max_pages;i++){
        if(coremap[i].state == free){
            int freepages = 1;
            for(int j = 0;j < (int)npages-1;j++){
                if(coremap[i].state != free){
                    break;
                }
                freepages++;
            }
            if(freepages == (int)npages){
                for(int j = i+1;j < (int)npages;j++){
                    coremap[j].state = dirty;
                    coremap[i].nsize = 0;
                }
                coremap[i].state = dirty;
                coremap[i].nsize = npages*PAGE_SIZE;
                coremap[i].vaddr = PADDR_TO_KVADDR(i*PAGE_SIZE);
                index = i;
                break;
            }
        }
    }
    if(index == 0){
        return 0;
    }
    
	coremap_bytes += (npages*PAGE_SIZE);
	
	return coremap[index].vaddr;
}

void free_kpages(vaddr_t vaddr){
    if(bootStrapped){
        paddr_t paddr = KVADDR_TO_PADDR(vaddr);
        int index = paddr/PAGE_SIZE;
        
        if(coremap[index].vaddr == vaddr){
			if(coremap[index].state != free && coremap[index].nsize != 0){
				int pages = coremap[index].nsize/PAGE_SIZE;
				for(int i = 0;i < pages;i++){
					coremap[index+i].state = free;
				}
				coremap_bytes -= coremap[index].nsize;
				coremap[index].nsize = 0;
			}
        }
    }
}

__size_t coremap_used_bytes(void){
	return coremap_bytes;
}

void vm_tlbshootdown_all(void){
	
}

void vm_tlbshootdown(const struct tlbshootdown *ts){
	(void)ts;
}