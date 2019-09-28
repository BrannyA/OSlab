#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;

extern queue_t ready_queue;
extern queue_t block_queue;
extern queue_t sleep_queue;

static void check_sleeping()
{
}

void scheduler(void)
{
    //printk("in scheduler\n");
    // TODO schedule
    // Modify the current_running pointer.
    if(current_running == NULL)
    {
    //    printk("in if\n");
        current_running = queue_dequeue(&ready_queue);
        current_running->status = TASK_RUNNING;
        process_id = current_running->pid;
    }
    else
    {
    //    printk("%d", sizeof(pcb_t));
    //    printk("in else\n");
    //    printk("push: %x pid %d\n", (int)current_running, current_running->pid);
    //    printk("%x\n", (int)current_running->user_context.regs[31]);
        queue_push(&ready_queue, current_running);
        current_running->status = TASK_READY;
        current_running = queue_dequeue(&ready_queue);
        // printk("dequeue: %x\n", (int)current_running);
        // printk("dequeue2: %x\n", (int)&(*current_running));
        // printk("%x\n", (int)current_running->user_context.regs[31]);
        current_running->status = TASK_RUNNING;
        process_id = current_running->pid;
        // printk("sche done\n");
  }
  return;
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
}
