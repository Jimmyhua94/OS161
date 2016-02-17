/*
 * All the contents of this file are overwritten during automated
 * testing. Please consider this before changing anything in this file.
 */

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <kern/secret.h>
#include <spinlock.h>

#define NTHREADS      32

static volatile unsigned long testval1;
static volatile unsigned long testval2;

static struct rwlock *testrw = NULL;
struct spinlock status_lock;
static struct semaphore *donesem = NULL;

static bool test_status = FAIL;

static
void
rtestthread(void *junk, unsigned long num)
{
	(void)junk;
    (void)num;
    kprintf_n("read thread start...\n");
    //spinlock_acquire(&status_lock);
    rwlock_acquire_read(testrw);
    //spinlock_release(&status_lock);
    kprintf_n("enter read...\n");
    if (testval1 != testval2)
        goto fail;
    rwlock_release_read(testrw);
    kprintf_n("read thread end...\n");
	V(donesem);
    return;

fail:
    rwlock_release_read(testrw);
	test_status = FAIL;;
	V(donesem);
	return;
}

static
void
wtestthread(void *junk, unsigned long num)
{
	(void)junk;
    (void)num;
    kprintf_n("write thread start...\n");
    //spinlock_acquire(&status_lock);
    rwlock_acquire_write(testrw);
    //spinlock_release(&status_lock);
    kprintf_n("write thread acquired...\n");
    testval1++;
    if (testval1 != ++testval2){
        kprintf_n("testval != testval2...\n");
        goto fail;
    }
    rwlock_release_write(testrw);
    kprintf_n("write thread end...\n");
	V(donesem);
    return;

fail:
    rwlock_release_write(testrw);
    kprintf_n("Failed...\n");
	test_status = FAIL;
	V(donesem);
	return;
}

int
rwtest(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	int i, result;

	kprintf_n("Starting rwt1...\n");
	
	test_status = SUCCESS;
    
    testrw = rwlock_create("rwtest");
    if (testrw == NULL) {
		panic("rwt1: rwlock_create failed\n");
	}
    kprintf_n("rwlock created...\n");
    donesem = sem_create("donesem", 0);
	if (donesem == NULL) {
		panic("rwt1: sem_create failed\n");
	}

    spinlock_init(&status_lock);

	testval1 = NTHREADS-1;
    testval2 = testval1;
    
	for (i=0; i<NTHREADS/2; i++) {
		kprintf_t(".");
		result = thread_fork("rwtest", NULL, wtestthread, NULL, (long unsigned) i);
		if (result) {
			panic("rwt1: thread_fork failed: %s\n", strerror(result));
		}
	}
    
	for (i=0; i<NTHREADS/2; i++) {
		kprintf_t(".");
		result = thread_fork("rwtest", NULL, rtestthread, NULL, (long unsigned) i);
		if (result) {
			panic("rwt1: thread_fork failed: %s\n", strerror(result));
		}
	}
    
	for (i=0; i<NTHREADS; i++) {
		kprintf_t(".");
		P(donesem);
	}
    
    kprintf_n("end rwt1...\n");
    
    rwlock_destroy(testrw);
    sem_destroy(donesem);
    testrw = NULL;
    donesem = NULL;
    
    kprintf_n("clear rwt1...\n");

	kprintf_t("\n");
	success(test_status, SECRET, "rwt1");

	return 0;
}
