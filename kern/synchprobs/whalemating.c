/*
 * Copyright (c) 2001, 2002, 2009
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

/*
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */

static struct semaphore *male_sem;
static struct semaphore *female_sem;
static struct semaphore *matchmaker_sem;
static struct semaphore *mating_sem;
static volatile int male_count;
static volatile int female_count;
static struct lock *male_lock;
static  struct lock *female_lock;


void whalemating_init() {
    male_sem = sem_create("male_sem",0);
    if (male_sem == NULL) {
        panic("sp1: sem_create failed\n");
    }
    female_sem = sem_create("female_sem",0);
    if (female_sem == NULL) {
        panic("sp1: sem_create failed\n");
    }
    matchmaker_sem = sem_create("matchmaker_sem",0);
    if (matchmaker_sem == NULL) {
        panic("sp1: sem_create failed\n");
    }
    mating_sem = sem_create("mating_sem",0);
    if (mating_sem == NULL) {
        panic("sp1: sem_create failed\n");
    }
    
    male_lock = lock_create("male_lock");
    
    female_lock = lock_create("female_lock");
    
	return;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
    sem_destroy(male_sem);
    sem_destroy(female_sem);
    sem_destroy(matchmaker_sem);
    sem_destroy(mating_sem);
    
    lock_destroy(male_lock);
    lock_destroy(female_lock);
	return;
}

void
male(uint32_t index)
{
    uint32_t male_id = index;
    male_start(male_id);
    lock_acquire(male_lock);
    male_count++;
    lock_release(male_lock);
    P(male_sem);
    male_end(male_id);
    lock_acquire(male_lock);
    male_count--;
    lock_release(male_lock);
	/*
	 * Implement this function by calling male_start and male_end when
	 * appropriate.
	 */
	return;
}

void
female(uint32_t index)
{
    uint32_t female_id = index;
    female_start(female_id);
    lock_acquire(female_lock);
    female_count++;
    lock_release(female_lock);
    P(female_sem);
    female_end(female_id);
    lock_acquire(female_lock);
    female_count--;
    lock_release(female_lock);

	/*
	 * Implement this function by calling female_start and female_end when
	 * appropriate.
	 */
	return;
}

void
matchmaker(uint32_t index)
{
    uint32_t matchmaker_id = index;
    matchmaker_start(matchmaker_id);
    do{
        if(male_count > 0 && female_count > 0){
            V(male_sem);
            V(female_sem);
        }
    }while(male_count == 0 || female_count == 0);
    matchmaker_end(matchmaker_id);

	/*
	 * Implement this function by calling matchmaker_start and matchmaker_end
	 * when appropriate.
	 */
	return;
}
