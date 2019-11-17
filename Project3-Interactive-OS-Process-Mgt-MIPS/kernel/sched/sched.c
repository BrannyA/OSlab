#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;
// queue_t ready_queue_1;
// queue_t ready_queue_2;
// queue_t ready_queue_3;
/* global process id */
pid_t process_id = 0;

// void priority_queue_push(pcb_t *cur_running)
// {
//     int pri = cur_running->priority;
//     int task_priority = cur_running->task_priority;
//     if(task_priority == 1)
//     {
//         if(pri == 1)
//         {
//             queue_push(&ready_queue_1, cur_running);
//             cur_running->task_priority = 1;
//         }
//         else if(pri == 2)
//         {
//             queue_push(&ready_queue_2, cur_running);
//             cur_running->task_priority = 2;
//         }
//         else
//         {
//             queue_push(&ready_queue_3, cur_running);
//             cur_running->task_priority = 3;
//         }
//     }
//     else if(task_priority == 2)
//     {
//         queue_push(&ready_queue_1, cur_running);
//         cur_running->task_priority = 1;
//     }
//     else if(task_priority == 3)
//     {
//         cur_running->task_priority = 2;
//         queue_push(&ready_queue_2, cur_running);
//     }
// }
//
// void *priority_dequeue()
// {
//     pcb_t *cur;
//     if(!queue_is_empty(&ready_queue_3))
//         cur = queue_dequeue(&ready_queue_3);
//     else if(!queue_is_empty(&ready_queue_2))
//         cur = queue_dequeue(&ready_queue_2);
//     else if(!queue_is_empty(&ready_queue_1))
//         cur = queue_dequeue(&ready_queue_1);
//     return (void *)cur;
// }

static void check_sleeping()
{
    pcb_t *p = sleep_queue.head;
    uint32_t time = get_timer();
    while(p != NULL)
    {
        if(time >= p->wakeup_time)
        {
            pcb_t *q = (pcb_t *)queue_remove(&sleep_queue, (void *)p);
            p->wakeup_time = 0;
            p->status = TASK_READY;
            p->in_queue = &ready_queue;
            queue_push(&ready_queue, (void *)p);
            // _priority_queue_push(&ready_queue, (void *)p);
            p = q;
        }
        else
            p = p->next;
    }
}

void scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
    check_sleeping();
    pcb_t *next_running, *p;
    if(queue_is_empty(&ready_queue))
        next_running = current_running;
    else
        next_running = (pcb_t *)queue_dequeue(&ready_queue);

    if(current_running->status != TASK_BLOCKED &&
        current_running->status != TASK_EXITED &&
        next_running != current_running)
    {
        current_running->status = TASK_READY;
        current_running->in_queue = &ready_queue;
        if(current_running->pid != 0)
            queue_push(&ready_queue, current_running);
        // _priority_queue_push(&ready_queue, current_running);
    }
    current_running = next_running;
    current_running->priority = current_running->task_priority;
    current_running->status = TASK_RUNNING;
    current_running->in_queue = NULL;
    //current_running->priority-=1;
    p = (pcb_t *)ready_queue.head;
    while(p != NULL)
    {
        p->priority += 1;
        p = p->next;
    }
    // printk("after scheduler, pid = %d", current_running->pid);
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
    current_running->status = TASK_BLOCKED;
    current_running->wakeup_time = get_timer()+sleep_time;
    current_running->in_queue = &sleep_queue;
    queue_push(&sleep_queue, (void *)current_running);
    do_scheduler();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
    current_running->status = TASK_BLOCKED;
    current_running->in_queue = queue;
    queue_push(queue, (void *)current_running);
    // _priority_queue_push(queue, (void *)current_running);
    do_scheduler();
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
    // pcb_t *p = (pcb_t *)(queue->head);
    // while(p != NULL)
    // {
    //     p->priority += 1;
    //     p = p->next;
    // }

    pcb_t *item = (pcb_t *)queue_dequeue(queue);
    item->status = TASK_READY;
    item->in_queue = &ready_queue;
    queue_push(&ready_queue, item);
    // _priority_queue_push(&ready_queue, item);
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
    pcb_t *item;
    while(!queue_is_empty(queue))
    {
        item = (pcb_t *)queue_dequeue(queue);
        item->status = TASK_READY;
        item->in_queue = &ready_queue;
        queue_push(&ready_queue, item);
        // _priority_queue_push(&ready_queue, item);
    }
}



void do_ps(int *location)
{
    int i, j;

    vt100_move_cursor(1, *location + 1);
    printk("[PROCESS TABLE]\n");
    for(i = 0, j = 0; i < NUM_MAX_TASK; i++)
    {
        if(pcb[i].status == TASK_RUNNING)
        {
            *location = *location + 1;
            vt100_move_cursor(1, *location+1);
            printk("[%d] PID : %d STATUS : RUNNING\n", j, pcb[i].pid);
            j++;
        }
        else if(pcb[i].status == TASK_READY)
        {
            *location = *location + 1;
            vt100_move_cursor(1, *location+1);
            printk("[%d] PID : %d STATUS : READY\n", j, pcb[i].pid);
            j++;
        }
        else if(pcb[i].status == TASK_BLOCKED)
        {
            *location = *location + 1;
            vt100_move_cursor(1, *location+1);
            printk("[%d] PID : %d STATUS : BLOCKED\n", j, pcb[i].pid);
            j++;
        }
    }
  // for(i = 0; i <= 5; i++)
  // {
  //     *location = *location + 1;
  //     vt100_move_cursor(1, *location+1);
  //     printk("[%d] PID : %d STATUS : %d\n", i, pcb[i].pid, pcb[i].status);
  // }
}

pid_t do_getpid()
{
    return current_running->pid;
}

void do_spawn(task_info_t * task)
{
    int i;
    for(i = 0; i < NUM_MAX_TASK; i++)
    {
        if(pcb[i].status == TASK_EXITED)
            break;
    }

    if(i == NUM_MAX_TASK)
    {
         printk("Have no available pcb");
         while(1) ;
    }
    init_one_pcb(&pcb[i], task); //& push to ready_queue
}

void do_exit()
{
    current_running->status = TASK_EXITED;
    current_running->in_queue = NULL;
    // relaese locks
    int i;
    for(i = 0; i <= current_running->lock_top; i++)
        do_mutex_lock_release(current_running->lock[i]);
    // relaese wait queue
    while(!queue_is_empty(&current_running->wait_queue))
    {
        pcb_t *wait_task = queue_dequeue(&current_running->wait_queue);
        wait_task->status = TASK_READY;
        wait_task->in_queue = &ready_queue;
        queue_push(&ready_queue, (void *)wait_task);
    }
    do_scheduler();
}

void do_wait(pid_t pid)
{
    int i;
    for(i = 0; i < NUM_MAX_TASK; i++)
    {
        if(pcb[i].pid == pid)
            break;
    }
    if(i == NUM_MAX_TASK)
    {
        screen_move_cursor(0, screen_cursor_y);
        printk("\nWaiting task is not exist! pid = %d  cur = %d         \n", pid, current_running->pid);
    }
    else
    {
        current_running->status = TASK_BLOCKED;
        current_running->in_queue = &pcb[i].wait_queue;
        queue_push(&pcb[i].wait_queue, (void *)current_running);
        do_scheduler();
    }
}

void do_kill(pid_t pid)
{
    int i;
    pcb_t *to_kill;
    // find the corresopnding pcb
    for(i = 0; i < NUM_MAX_TASK; i++)
    {
        if(pcb[i].pid == pid )//&& pcb[i].status != TASK_EXITED)
            break;
    }
    // vt100_move_cursor(1, 20);
    // printk("here pcb[%d].pid = %d\n", i, pcb[i].pid);
    // if(pid == current_running->pid || i == NUM_MAX_TASK)
    //     return;
    to_kill = &pcb[i];
    //remove to_kill from other's queue
    if(to_kill->in_queue != NULL)
    {
        queue_remove(to_kill->in_queue, (void *)to_kill);
        to_kill->in_queue = NULL;
    }
    // relaese lock
    for(i = 0; i <= to_kill->lock_top; i++)
        do_mutex_lock_release(to_kill->lock[i]);
    // relaese wait queue
    while(!queue_is_empty(&to_kill->wait_queue))
    {
        pcb_t *wait_task = queue_dequeue(&to_kill->wait_queue);
        wait_task->status = TASK_READY;
        wait_task->in_queue = &ready_queue;
        queue_push(&ready_queue, (void *)wait_task);
    }
    to_kill->status = TASK_EXITED;
    // pcb_t *p = (pcb_t *)queue->head;
    // while(p->next != NULL)
    // {
    //     if(p == item)
    //         queue_remove(queue, item);
    // }
    do_scheduler();
}
