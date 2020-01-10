#include "irq.h"
#include "lock.h"
#include "time.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

pcb_t *current_running;

pid_t process_id = 0;

queue_t First_ready_queue;
queue_t Second_ready_queue;
queue_t Third_ready_queue;
queue_t Forth_ready_queue;
queue_t Fifth_ready_queue;

queue_t sleep_queue;

void switch_current_running() {
    do {
        if (!queue_is_empty(&First_ready_queue))
            current_running = queue_dequeue(&First_ready_queue);
        else if (!queue_is_empty(&Second_ready_queue))
            current_running = queue_dequeue(&Second_ready_queue);
        else if (!queue_is_empty(&Third_ready_queue))
            current_running = queue_dequeue(&Third_ready_queue);
        else if (!queue_is_empty(&Forth_ready_queue))
            current_running = queue_dequeue(&Forth_ready_queue);
        else if (!queue_is_empty(&Fifth_ready_queue))
            current_running = queue_dequeue(&Fifth_ready_queue);
        else {
            printk("NO PROCESS TO RUN");
            while (1);
        }
    } while (current_running->status == TASK_EXITED);

    current_running->status = TASK_RUNNING;
    process_id = current_running->pid;
}

void push_to_ready_queue(pcb_t *PCB) {
    PCB->status = TASK_READY;
    if (PCB->priority == FISRT_PRIORITY) {
        queue_push(&Second_ready_queue, PCB);
        PCB->priority = SECOND_PRIORITY;
    } else if (PCB->priority == SECOND_PRIORITY) {
        queue_push(&Third_ready_queue, PCB);
        PCB->priority = THIRD_PRIORITY;
    } else if (PCB->priority == THIRD_PRIORITY) {
        queue_push(&Forth_ready_queue, PCB);
        PCB->priority = FORTH_PRIORITY;
    } else if (PCB->priority == FORTH_PRIORITY) {
        queue_push(&Fifth_ready_queue, PCB);
        PCB->priority = FIFTH_PRIORITY;
    } else
        queue_push(&Fifth_ready_queue, PCB);
}

static void check_sleeping() {
    pcb_t *PCB;
    pcb_t *next = sleep_queue.head;

    uint32_t current_time = get_timer();

    if (!queue_is_empty(&sleep_queue)) {
        do {
            PCB = next;

            if (current_time >= PCB->sleep_deadline) {
                next = queue_remove(&sleep_queue, PCB);
                if (PCB->status != TASK_EXITED)
                    push_to_ready_queue(PCB);
            } else
                next = PCB->next;
        } while (next != NULL);
    }
}

void scheduler(void) {
    // save the cursor
    current_running->cursor_x = screen_cursor_x;
    current_running->cursor_y = screen_cursor_y;

    check_sleeping();

    if (current_running == NULL || current_running->status != TASK_RUNNING)
        switch_current_running();
    else {
        push_to_ready_queue(current_running);
        switch_current_running();
    }

    // restore the cursor
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;

    // set EntryHI_ASID
    set_EntryHi(current_running->pid);

    return;
}

void do_sleep(uint32_t sleep_time) {
    queue_push(&sleep_queue, current_running);
    current_running->status = TASK_SLEEPING;
    current_running->sleep_deadline = get_timer() + sleep_time;
    do_scheduler();
}

void do_block(queue_t *queue) {
    // block the current_running task into the queue
    queue_push(queue, current_running);
    current_running->status = TASK_BLOCKED;
    do_scheduler();
}

void do_unblock_one(queue_t *queue) {
    // unblock the head task from the queue
    pcb_t *unblocked;
    if (!queue_is_empty(queue)) {
        unblocked = queue_dequeue(queue);
        if (unblocked->status != TASK_EXITED)
            push_to_ready_queue(unblocked);
    }
}

void do_unblock_all(queue_t *queue) {
    while (!queue_is_empty(queue)) {
        pcb_t *ready_pcb = (pcb_t *) queue_dequeue(queue);
        if (ready_pcb->status != TASK_EXITED)
            push_to_ready_queue(ready_pcb);
    }
}

void do_ps(int *location) {
    int i, j;

    vt100_move_cursor(1, *location + 1);
    printk("[PROCESS TABLE]\n");

    for (i = 0, j = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].status == TASK_RUNNING) {
            *location = *location + 1;
            vt100_move_cursor(1, *location + 1);
            printk("[%d] PID : %d STATUS : RUNNING\n", j, pcb[i].pid);
            j++;
        } else if (pcb[i].status == TASK_READY) {
            *location = *location + 1;
            vt100_move_cursor(1, *location + 1);
            printk("[%d] PID : %d STATUS : READY\n", j, pcb[i].pid);
            j++;
        }
    }
}

pid_t do_getpid() {
    return current_running->pid;
}

void give_pcb(pcb_t *pcb, task_info_t *task, int i) {
    static next_pid = 1;

    int j;
    for (j = 0; j < 31; j++) {
        pcb->kernel_context.regs[j] = 0;
        pcb->user_context.regs[j] = 0;
    }
    pcb->kernel_context.regs[31] = (uint32_t) handle_int + 0x14;
    pcb->user_context.regs[31] = 0;

    pcb->user_context.regs[29] = STACK_TOP_U;
    pcb->kernel_context.regs[29] = STACK_TOP_K - i * STACK_SIZE;

    pcb->user_context.cp0_status = 0x10008000;
    pcb->user_context.hi = 0;
    pcb->user_context.lo = 0;
    pcb->user_context.cp0_badvaddr = 0;
    pcb->user_context.cp0_cause = 0;
    pcb->user_context.cp0_epc = task->entry_point;
    pcb->user_context.pc = 0;
    pcb->user_stack_top = STACK_TOP_U;

    pcb->kernel_context.cp0_status = 0x10008000;
    pcb->kernel_context.hi = 0;
    pcb->kernel_context.lo = 0;
    pcb->kernel_context.cp0_badvaddr = 0;
    pcb->kernel_context.cp0_cause = 0;
    pcb->kernel_context.cp0_epc = 0;
    pcb->kernel_context.pc = 0;
    pcb->kernel_stack_top = STACK_TOP_K - i * STACK_SIZE;

    pcb->pid = next_pid;
    pcb->type = task->type;

    queue_init(&pcb->wait_queue);
    queue_init(&pcb->lock_queue);

    pcb->status = TASK_READY;
    pcb->priority = FISRT_PRIORITY;
    queue_push(&First_ready_queue, pcb);

    next_pid++;
}

void do_spawn(task_info_t *task) {
    int i;

    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].status == TASK_EXITED)
            break;
    }

    if (i == NUM_MAX_TASK) {
        printk("Have no available pcb");
        while (1);
    }

    give_pcb(&pcb[i], task, i); //& push to ready_queue
}

void do_exit() {
    current_running->status = TASK_EXITED;

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

    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED) {
            current_running->status = TASK_BLOCKED;
            queue_push(&pcb[i].wait_queue, current_running);
            do_scheduler();
            return;
        }
    }
}

void do_kill(pid_t pid) {
    int i, j;


    // find the corresopnding pcb
    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED)
            break;
    }

    if (pid == current_running->pid || i == NUM_MAX_TASK)
        return;

    // relaese it's wait queue
    do_unblock_all(&pcb[i].wait_queue);

    // relaese all locks the process has
    while (!queue_is_empty(&pcb[i].lock_queue)) {
        mutex_lock_t *lock = ((mutex_lock_t *) queue_dequeue(&pcb[i].lock_queue));
        pcb_t *pcb;

        do_unblock_all(&lock->block_queue);
        lock->status = UNLOCKED;
    }

    pcb[i].status = TASK_EXITED;
    do_scheduler();
}
