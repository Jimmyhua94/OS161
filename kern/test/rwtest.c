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
#include <kern/test161.h>
#include <spinlock.h>

#define NTHREADS      32

static volatile unsigned long testval1;
static volatile unsigned long testval2;

static struct rwlock *testrw = NULL;
struct spinlock status_lock;
static struct semaphore *donesem = NULL;

static bool test_status = TEST161_FAIL;

static
void
rtestthread(void *junk, unsigned long num)
{
	(void)junk;
    (void)num;
    kprintf_n("read thread start...\n");
    rwlock_acquire_read(testrw);
    kprintf_n("read thread acquired...\n");
    kprintf_t("read testval1: %lu\n",testval1);
    kprintf_t("read testval2: %lu\n",testval2);
    if (testval1 != testval2)
        goto fail;
    rwlock_release_read(testrw);
    kprintf_n("read thread released...\n");
	V(donesem);
    return;

fail:
    rwlock_release_read(testrw);
	test_status = TEST161_FAIL;
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
    rwlock_acquire_write(testrw);
    kprintf_n("write thread acquired...\n");
    kprintf_t("testval1: %lu\n",testval1);
    testval1++;
    kprintf_t("write testval1++: %lu\n",testval1);
    if (testval1 != ++testval2){
        goto fail;
    }
    rwlock_release_write(testrw);
    kprintf_n("write thread released...\n");
	V(donesem);
    return;

fail:
    rwlock_release_write(testrw);
	test_status = TEST161_FAIL;
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
	
	test_status = TEST161_SUCCESS;
    
    testrw = rwlock_create("rwtest");
    if (testrw == NULL) {
		panic("rwt1: rwlock_create failed\n");
	}
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
    
    rwlock_destroy(testrw);
    sem_destroy(donesem);
    testrw = NULL;
    donesem = NULL;

	kprintf_t("\n");
	success(test_status, SECRET, "rwt1");

	return 0;
}

int rwtest2(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt2 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt2");

	return 0;
}

int rwtest3(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt3 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt3");

	return 0;
}

int rwtest4(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt4 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt4");

	return 0;
}

int rwtest5(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt5 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt5");

	return 0;
}
