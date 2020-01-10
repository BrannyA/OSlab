#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;
pcb_t *pcb_shift;

/* global process id */
pid_t process_id = 1;

static void check_sleeping() {
    int pcb_num;
    if (!queue_is_empty(&sleep_queue))
        for (pcb_shift = sleep_queue.head, pcb_num = 0;
             pcb_shift != NULL; pcb_shift = pcb_shift->next, pcb_num = pcb_num + 1);
    for (; pcb_num > 0; pcb_num = pcb_num - 1) {
        pcb_shift = (pcb_t *) queue_dequeue(&sleep_queue);
        if (get_timer() - pcb_shift->start_time >= pcb_shift->sleeping_time) {
            if (pcb_shift->status != TASK_EXITED) {
                pcb_shift->status = TASK_READY;
                queue_push(&ready_queue, pcb_shift);
            }
        } else
            queue_push(&sleep_queue, pcb_shift);
    }
}

void do_process_show() {
    int i, j;
    //sys_move_cursor(screen_cursor_x, screen_cursor_y);
    for (i = 0, j = 0; i < 16; i++) {
        if (pcb[i].pcb_using == 1) {
            do_printf("[%d] pid:%d ", j, pcb[i].pid);
            j++;
            if (pcb[i].status == TASK_READY)
                do_printf("STATUS:READY\n");
            else if (pcb[i].status == TASK_BLOCKED)
                do_printf("STATUS:BLOCKED\n");
            else if (pcb[i].status == TASK_RUNNING)
                do_printf("STATUS:RUNNING\n");
            else if (pcb[i].status == TASK_SLEEPING)
                do_printf("STATUS:SLEEPING\n");
            else if (pcb[i].status == TASK_WAITING)
                do_printf("STATUS:WAITING\n");
            else if (pcb[i].status == TASK_EXITED)
                do_printf("STATUS:EXITED\n");
        }
    }
}

void scheduler(void) {
    screen_reflush();
    current_running->cursor_x = screen_cursor_x;
    current_running->cursor_y = screen_cursor_y;

    if (current_running->status == TASK_RUNNING) {
        current_running->status = TASK_READY;

        priority_push(&ready_queue, current_running);//放入READY队列末，等待执行
    } else if (current_running->status == TASK_BLOCKED)//当前线程被阻塞
    { ;
    } else if (current_running->status == TASK_SLEEPING) { ;
    } else if (current_running->status == TASK_WAITING) { ;
    }
    do {
        current_running = (pcb_t *) queue_dequeue(&ready_queue);//从READY队列头取得一个线程的信息
    } while (current_running->status == TASK_EXITED);
    current_running->status = TASK_RUNNING;
    check_sleeping();
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
    set_EntryHi(current_running->pid);

}

void do_sleep(uint32_t sleep_time) {
    // TODO sleep(seconds)
    if (current_running->status != TASK_EXITED) {
        current_running->status = TASK_SLEEPING;
        current_running->sleeping_time = sleep_time;
        current_running->start_time = get_timer();
        queue_push(&sleep_queue, (void *) current_running);
    }
    do_scheduler();
}

void do_block(queue_t *queue) {
    // block the current_running task into the queue
    current_running->status = TASK_BLOCKED;
    queue_push(queue, (void *) current_running);
    do_scheduler();
}

void do_unblock_one(queue_t *queue) {
    // unblock the head task from the queue
    pcb_t *unblockone;
    if (!queue_is_empty(queue)) {
        do {
            unblockone = (pcb_t *) queue_dequeue(queue);
        } while (unblockone->status == TASK_EXITED);
        unblockone->status = TASK_READY;
        queue_push(&ready_queue, unblockone);
    }
}

void do_unblock_all(queue_t *queue) {
    // unblock all task in the queue
    pcb_t *unblockall;
    while (!queue_is_empty(queue)) {
        unblockall = (pcb_t *) queue_dequeue(queue);
        if (unblockall->status != TASK_EXITED) {
            unblockall->status = TASK_READY;
            queue_push(&ready_queue, unblockall);
        }

    }
}

void do_spawn(task_info_t *task, unsigned long *argv) {
    int i, j;
    for (i = 0; pcb[i].pcb_using == 1; i++);
    pcb[i].kernel_context.regs[29] = 0xa0f00000 - 0x2000 * i;//分配栈
    pcb[i].user_context.regs[29] = 0xa0f00000 - 0x2000 * (i + 16);//分配栈
    pcb[i].kernel_context.regs[31] = (uint32_t)&first_run;
    pcb[i].kernel_context.cp0_epc = (uint32_t) task->entry_point;
    pcb[i].user_context.regs[31] = (uint32_t) task->entry_point;
    pcb[i].user_context.cp0_epc = (uint32_t) task->entry_point;
    pcb[i].kernel_context.cp0_status = 0x1000fc03;
    pcb[i].user_context.cp0_status = 0x1000fc03;
    pcb[i].status = TASK_READY;
    pcb[i].pid = process_id;
    process_id = process_id + 1;
    pcb[i].pcb_using = 1;
    pcb[i].cursor_x = 0;
    pcb[i].cursor_y = 0;
    pcb[i].waiting_time = 0;
    pcb[i].priority = 1;
    pcb[i].user_context.regs[4] = (uint32_t) argv;
    priority_push(&ready_queue, &pcb[i]);

}

void do_kill(pid_t pid) {
    int i, j;


    // find the corresopnding pcb
    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED)
            break;
    }

    do_unblock_all(&pcb[i].wait_queue);

    while (!queue_is_empty(&pcb[i].lock_queue)) {
        mutex_lock_t *lock = ((mutex_lock_t *) queue_dequeue(&pcb[i].lock_queue));
        pcb_t *pcb;

        do_unblock_all(&lock->block_queue);
        lock->status = UNLOCKED;
    }

    pcb[i].status = TASK_EXITED;
    pcb[i].pcb_using = 0;
    do_scheduler();

}

void do_exit() {
    current_running->status = TASK_EXITED;
    current_running->pcb_using = 0;

    // relaese it's wait queue
    do_unblock_all(&current_running->wait_queue);

    // relaese all locks the process has
    while (!queue_is_empty(&current_running->lock_queue)) {
        mutex_lock_t *lock = ((mutex_lock_t *) queue_dequeue(&current_running->lock_queue));
        pcb_t *pcb;

        do_unblock_all(&lock->block_queue);
        lock->status = UNLOCKED;
    }

    do_scheduler();
}

void do_wait(pid_t pid) {
    int i;
    if (current_running->pid == pid) {
        return NULL;
    }
    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED) {
            current_running->status = TASK_BLOCKED;
            queue_push(&pcb[i].wait_queue, current_running);
            do_scheduler();
            return;
        }
    }
}

pid_t do_getpid() {
    int getpid = current_running->pid;
    pidget(getpid);
}
