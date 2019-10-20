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

/* queue */
extern queue_t ready_queue_3;
extern queue_t ready_queue_2;
extern queue_t ready_queue_1;
extern queue_t block_queue;

static void check_sleeping()
{
}

void priority_queue_push(pcb_t *cur_running)
{
    int pri = cur_running->priority;
    int cur_pri = cur_running->cur_pri;
    if(cur_pri == PRIORITY_1)
    {
        if(pri == PRIORITY_1)
        {
            queue_push(&ready_queue_1, cur_running);
            cur_running->cur_pri = PRIORITY_1;
        }
        else if(pri == PRIORITY_2)
        {
            queue_push(&ready_queue_2, cur_running);
            cur_running->cur_pri = PRIORITY_2;
        }
        else
        {
            queue_push(&ready_queue_3, cur_running);
            cur_running->cur_pri = PRIORITY_3;
        }
    }
    else if(cur_pri == PRIORITY_2)
    {
        queue_push(&ready_queue_1, cur_running);
        cur_running->cur_pri = PRIORITY_1;
    }
    else if(cur_pri == PRIORITY_3)
    {
        cur_running->cur_pri = PRIORITY_2;
        queue_push(&ready_queue_2, cur_running);
    }
}

void *priority_dequeue()
{
    pcb_t *cur;
    if(!queue_is_empty(&ready_queue_3))
        cur = queue_dequeue(&ready_queue_3);
    else if(!queue_is_empty(&ready_queue_2))
        cur = queue_dequeue(&ready_queue_2);
    else if(!queue_is_empty(&ready_queue_1))
        cur = queue_dequeue(&ready_queue_1);
    return (void *)cur;
}

void scheduler(void)
{
    // printk("pid = %d\n", current_running->pid);
    current_running->cursor_x = screen_cursor_x;
    current_running->cursor_y = screen_cursor_y;

    if(current_running->status != TASK_BLOCKED && current_running->pid != 0)
    {
        // queue_push(&ready_queue, current_running);
        priority_queue_push(current_running);
        current_running->status = TASK_READY;
    }
    // current_running = queue_dequeue(&ready_queue);
    current_running = priority_dequeue();
    current_running->status = TASK_RUNNING;
    process_id = current_running->pid;

    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
 }

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
    current_running->status = TASK_BLOCKED;
    queue_push(queue, current_running);
    enable_interrupt();
    while(current_running->status == TASK_BLOCKED)
    {

    }
    //do_scheduler();
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
    pcb_t *awake = queue_dequeue(queue);
    awake->status = TASK_READY;
    queue_push(&ready_queue_3, awake);
    enable_interrupt();
    //do_scheduler();
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
}
