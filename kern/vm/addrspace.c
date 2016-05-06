/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <spl.h>
#include <mips/tlb.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}
    
    as->pgt = kmalloc(sizeof(struct pgtentry));
    as->pgt->next = NULL;
    as->region = kmalloc(sizeof(struct region));
    as->region->next = NULL;

	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *new;

	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */
    struct region* newtemp = new->region;
    struct region* oldtemp = old->region;
    while(oldtemp->next != NULL){
        newtemp->next = kmalloc(sizeof(struct region));
        newtemp->next->start = oldtemp->next->start;
        newtemp->next->pages = oldtemp->next->pages;
        newtemp->next->permissions = oldtemp->next->permissions;
        newtemp->next->next = NULL;
        newtemp = newtemp->next;
        oldtemp = oldtemp->next;
    }
    
    struct pgtentry* pgtnew = new->pgt;
    struct pgtentry* pgtold = old->pgt;
    // lock_acquire(coremap_biglock);
    while(pgtold->next != NULL){
        pgtnew->next = kmalloc(sizeof(struct pgtentry));
        pgtnew->next->vpn = pgtold->next->vpn;
		vaddr_t pgvaddr = PADDR_TO_KVADDR(pgtold->next->ppn);
		int i = 0;
		for(i = fixed_pages;i < max_pages;i++){
			if (coremap[i].vaddr == pgvaddr){
				break;
			}
		}
        pgtnew->next->ppn = KVADDR_TO_PADDR(alloc_kpages(coremap[i].nsize/PAGE_SIZE));
		memcpy((void *)PADDR_TO_KVADDR(pgtnew->next->ppn),(void *)pgvaddr,coremap[i].nsize);
        pgtnew->next->permission = pgtold->next->permission;
        pgtnew->next->state = pgtold->next->state;
        pgtnew->next->heap = pgtold->next->heap;
        pgtnew->next->next = NULL;
        pgtnew = pgtnew->next;
        pgtold = pgtold->next;
    }
    // lock_release(coremap_biglock);
    
    new->heap_start=old->heap_start;
    new->heap_end=old->heap_end;

	*ret = new;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */
	
	struct pgtentry* temp = as->pgt;
	bool first = true;
    lock_acquire(coremap_biglock);
	do{
		as->pgt = temp;
		temp = as->pgt->next;
		if(!first){
			free_kpages(PADDR_TO_KVADDR(as->pgt->ppn));
		}
		else{
			first = false;
		}
		kfree(as->pgt);
	}while(temp != NULL);
    lock_release(coremap_biglock);
    
	struct region* r = as->region;
	do{
		as->region = r;
		r = as->region->next;
		kfree(as->region);
	}while(r != NULL);
    
    kfree(as);
}

void
as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}

	/*
	 * Write this.
	 */
	 
	int spl;

	/* Disable interrupts on this CPU while frobbing the TLB. */
	spl = splhigh();

	for (int i=0; i<NUM_TLB; i++) {
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */
	size_t npages;

	/* Align the region. First, the base... */
	memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = memsize / PAGE_SIZE;
	struct region* temp = as->region;
	while(temp->next != NULL){
        temp = temp->next;
		if(temp->start == vaddr){
			return 0;
		}
	}
	temp->next = kmalloc(sizeof(struct region));
	if(temp->next == NULL){
		return ENOMEM;
	}
	temp->next->start = vaddr;
	temp->next->pages = npages;
	temp->next->permissions = temp->next->permissions & readable & writeable & executable;
    temp->next->next = NULL;
	as->heap_start = as->heap_end = vaddr + npages*PAGE_SIZE;
	return 0;
}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */
	(void)as;
	// struct region* r = as->region;
	// struct pgtentry* pgt = as->pgt;
	
	// while(r->next != NULL){
		// while(pgt->next != NULL){
			// pgt = pgt->next;
		// }
		// pgt->next = kmalloc(sizeof(struct pgtentry));
		// pgt->next->vpn = alloc_kpages(as->region->pages);
		// pgt->next->ppn = KVADDR_TO_PADDR(pgt->next->vpn);
		// paddr = KVADDR_TO_PADDR(vaddr);
	// }
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */
	/* Initial user-level stack pointer */
	vaddr_t base = USERSTACK - 1024 * PAGE_SIZE;
    
    size_t memsize = 1024*PAGE_SIZE;
    
	size_t npages;

	/* Align the region. First, the base... */
	memsize += base & ~(vaddr_t)PAGE_FRAME;
	base &= PAGE_FRAME;

	/* ...and now the length. */
	memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = memsize / PAGE_SIZE;
	struct region* temp = as->region;
	while(temp->next != NULL){
        temp = temp->next;
		if(temp->start == base){
			return 0;
		}
	}
	temp->next = kmalloc(sizeof(struct region));
	if(temp->next == NULL){
		return ENOMEM;
	}
	temp->next->start = base;
	temp->next->pages = npages;
	temp->next->permissions = temp->next->permissions & 1 & 1 & 1;
    temp->next->next = NULL;
	*stackptr = USERSTACK;

	return 0;
}

