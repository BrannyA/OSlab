#include "lock.h"
#include "sched.h"
#include "syscall.h"

void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

void spin_lock_init(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    while (LOCKED == lock->status)
    {
        //do_scheduler();
    };
    lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
    disable_interrupt();
    lock->status = UNLOCKED;
    queue_init(&lock->block_queue);
    enable_interrupt();
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    disable_interrupt();
    if(lock->status == UNLOCKED)
    {
        lock->status = LOCKED;
        enable_interrupt();
    }
    else
    {
        do_block(&lock->block_queue);
    }
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    disable_interrupt();
    if(queue_is_empty(&lock->block_queue))
    {
        lock->status = UNLOCKED;
        enable_interrupt();
    }
    else
    {
        do_unblock_one(&lock->block_queue);
    }
}
