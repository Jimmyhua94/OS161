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
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction ( X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */

static struct lock *zero_lk;
static struct lock *one_lk;
static struct lock *two_lk;
static struct lock *three_lk;

static struct semaphore *count_sem;

void
stoplight_init() {
    zero_lk = lock_create("zero_lock");
    if (zero_lk == NULL) {
        panic("sp1: lock_create failed\n");
    }
    one_lk = lock_create("one_lock");
    if (one_lk == NULL) {
        panic("sp1: lock_create failed\n");
    }
    two_lk = lock_create("two_lock");
    if (two_lk == NULL) {
        panic("sp1: lock_create failed\n");
    }
    three_lk = lock_create("three_lock");
    if (three_lk == NULL) {
        panic("sp1: lock_create failed\n");
    }

    count_sem = sem_create("count_semaphore",3);
    if (count_sem == NULL) {
        panic("sp1: sem_create failed\n");
    }
    
	return;
}

/*
 * Called by the driver during teardown.
 */

void stoplight_cleanup() {
    lock_destroy(zero_lk);
    lock_destroy(one_lk);
    lock_destroy(two_lk);
    lock_destroy(three_lk);

    sem_destroy(count_sem);
    
	return;
}

//move is the moves it is going to take (either 2nd or 3rd) where turn is stright,right or left
int
quadrants(int direction, int move, int turn)
{
    switch(turn) {
        case 0: //Go stright
            return (direction+3)%4;
        case 1: //Turn right
            return direction;
        case 2: //Turn left
            if(move == 2)
                return (direction+3)%4;
            else if (move == 3)
                return (direction+2)%4;
            else    //Should never come here if code is unchanged
                kprintf_n("You entered wrong turns in quadrant planning");
            break;
        default:    //Should never come here if code is unchanged
            kprintf_n("You entered wrong turns in quadrant planning");
            break;
    }
    return 0;
}

struct lock*
getLock(int quadrant){
    switch(quadrant){
        case 0:
            return zero_lk;
        case 1:
            return one_lk;
        case 2:
            return two_lk;
        case 3:
            return three_lk;
        default:    //Should never come here if code is unchanged
            kprintf_n("Incorrect quadrant.");
            break;
    }
    return NULL;
}

void
turnright(uint32_t direction, uint32_t index)
{
	int move_one = (int)direction;
    
    P(count_sem);
    
    lock_acquire(getLock(move_one));
    inQuadrant(move_one,index);
    leaveIntersection(index);
    lock_release(getLock(move_one));
    
    V(count_sem);
	return;
}
void
gostraight(uint32_t direction, uint32_t index)
{
	int move_one = (int)direction;
    int move_two = quadrants(move_one,2,0);
    
    P(count_sem);
    
    lock_acquire(getLock(move_one));
    inQuadrant(move_one,index);
    lock_acquire(getLock(move_two));
    inQuadrant(move_two,index);
    lock_release(getLock(move_one));
    leaveIntersection(index);
    lock_release(getLock(move_two));
    
    V(count_sem);
    
	return;
}
void
turnleft(uint32_t direction, uint32_t index)
{
	int move_one = (int)direction;
    int move_two = quadrants(move_one,2,2);
    int move_three = quadrants(move_one,3,2);
    
    P(count_sem);
    
    lock_acquire(getLock(move_one));
    inQuadrant(move_one,index);
    lock_acquire(getLock(move_two));
    inQuadrant(move_two,index);
    lock_release(getLock(move_one));
    lock_acquire(getLock(move_three));
    inQuadrant(move_three,index);
    lock_release(getLock(move_two));
    leaveIntersection(index);
    lock_release(getLock(move_three));
    
    V(count_sem);
    
	return;
}
