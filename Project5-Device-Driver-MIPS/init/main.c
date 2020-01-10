/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"
#include "mm.h"
#include "mac.h"

queue_t ready_queue;
queue_t sleep_queue;
queue_t wait_queue;
uint32_t exception_handler[32];
uint8_t pframe[PF_NUMBER];

void first_run(void) {
    first_return();
}

static void init_page_table() {
    int i;

    for (i = 0; i < PTE_NUMBER; i++) {
        PTE[i].VPN = i;
        PTE[i].PFN = 0;
        PTE[i].pid = 0;
        PTE[i].valid = 0;
    }

    /*for (i = 0; i < PF_NUMBER; i++) {
        pframe[i] = i;
        queue_push(&PF, &pframe);
    }*/

    for (i = 0; i < 32; i++) // For safety reasons, initialize C, D, V, G, but not initialize VPN & PFN
    {
        set_EntryLo0(0x14);
        set_EntryLo1(0x14);
        set_cp0_Index(i);
        set_PageMask();
    }
    return;
}

static void init_memory() {
    init_page_table();
}

static void init_pcb() {
    int i, j, n;
    for (i = 0; i < 16; i++) {
        for (n = 0; n < 32; n++) {
            pcb[i].kernel_context.regs[n] = 0;
            pcb[i].user_context.regs[n] = 0;
        }
        pcb[i].kernel_context.regs[29] = 0xa0f00000 - 0x2000 * i;
        pcb[i].user_context.regs[29] = 0xa0f00000 - 0x2000 * (i + 16);
        pcb[i].kernel_context.regs[31] = (uint32_t)&first_run;
        pcb[i].kernel_stack_top = 0xa0f00000 - 0x2000 * i;
        pcb[i].user_stack_top = 0xa0f00000 - 0x2000 * (i + 16);
        pcb[i].status = TASK_EXITED;
        pcb[i].priority = 1;
        pcb[i].waiting_time = 0;
        pcb[i].pcb_using = 0;
        pcb[i].pid = 0;
        pcb[i].cursor_x = 0;
        pcb[i].cursor_y = 0;
    }
    pcb[1].kernel_context.cp0_epc = (uint32_t) & test_shell;
    pcb[1].user_context.regs[31] = (uint32_t) & test_shell;
    pcb[1].user_context.cp0_epc = (uint32_t) & test_shell;
    pcb[1].pid = process_id;
    process_id = process_id + 1;
    pcb[1].status = TASK_READY;
    pcb[1].type = USER_THREAD;
    pcb[0].priority = 1;
    pcb[1].priority = 1;
    pcb[0].pcb_using = 1;
    pcb[1].pcb_using = 1;
    queue_init(&ready_queue);
    queue_init(&sleep_queue);
    queue_push(&ready_queue, &pcb[1]);
    current_running = &pcb[0];
    current_running->status = TASK_RUNNING;

}

static void init_exception_handler() {
    int i = 0;
    for (i = 0; i < 32; i++) {
        exception_handler[i] = (uint32_t) & handle_other;
    }
    exception_handler[0] = (uint32_t) & handle_int;
    exception_handler[8] = (uint32_t) & handle_syscall;
    exception_handler[TLBL] = (uint32_t) & handle_tlb;
    exception_handler[TLBS] = (uint32_t) & handle_tlb;
}

static void init_exception() {

    memcpy(BEV0_EBASE + BEV0_OFFSET, &exception_handler_entry, (exception_handler_end - exception_handler_begin));
    memcpy(BEV0_EBASE, &TLBexception_handler_entry, (TLBexception_handler_end - TLBexception_handler_begin));
    init_exception_handler();
    init_exce();

}

static void init_syscall(void) {
    // init system call table.
    syscall[SYSCALL_SLEEP] = (int (*)())do_sleep;

    syscall[SYSCALL_BLOCK] = (int (*)())do_block;
    syscall[SYSCALL_UNBLOCK_ONE] = (int (*)())do_unblock_one;
    syscall[SYSCALL_UNBLOCK_ALL] = (int (*)())do_unblock_all;

    syscall[SYSCALL_WRITE] = (int (*)())screen_write;
    //(*syscall[SYSCALL_READ] =
    syscall[SYSCALL_CURSOR] = (int (*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH] = (int (*)())screen_reflush;
    syscall[SYSCALL_CLEAR] = (int (*)())screen_clear;

    syscall[SYSCALL_MUTEX_LOCK_INIT] = (int (*)())do_mutex_lock_init;
    syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = (int (*)())do_mutex_lock_acquire;
    syscall[SYSCALL_MUTEX_LOCK_RELEASE] = (int (*)())do_mutex_lock_release;

    syscall[SYSCALL_PROCESS_SHOW] = (int (*)())do_process_show;
    syscall[SYSCALL_SPAWN] = (int (*)())do_spawn;
    syscall[SYSCALL_KILL] = (int (*)())do_kill;
    syscall[SYSCALL_EXIT] = (int (*)())do_exit;
    syscall[SYSCALL_WAITPID] = (int (*)())do_wait;
    syscall[SYSCALL_GETPID] = (int (*)())do_getpid;

    // syscall[SYSCALL_CONDITION_INIT] = do_condition_init;
    // syscall[SYSCALL_CONDITION_WAIT] = do_condition_wait;
    // syscall[SYSCALL_CONDITION_SIGNAL] = do_condition_signal;
    // syscall[SYSCALL_CONDITION_BROADCAST] = do_condition_broadcast;
    //
    // syscall[SYSCALL_SEMAPHORE_INIT] = do_semaphore_init;
    // syscall[SYSCALL_SEMAPHORE_UP] = do_semaphore_up;
    // syscall[SYSCALL_SEMAPHORE_DOWN] = do_semaphore_down;
    //
    // syscall[SYSCALL_BARRIER_INIT] = do_barrier_init;
    // syscall[SYSCALL_BARRIER_WAIT] = do_barrier_wait;

    syscall[SYSCALL_INIT_MAC] = (int (*)()) &do_init_mac;
    syscall[SYSCALL_NET_RECV] = (int (*)()) &do_net_recv;
    syscall[SYSCALL_NET_SEND] = (int (*)()) &do_net_send;
    syscall[SYSCALL_WAIT_RECV_PACKAGE] = (int (*)()) &do_wait_recv_package;
}


// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void) {
    // Close the cache, no longer refresh the cache
    // when making the exception vector entry copy
    asm_start();

    // init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n");

    // init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n");


    init_memory();
    printk("> [INIT] Virtual memory initialization succeeded.\n");
    // init Process Control Block (-_-!)
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n");

    // init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n");
    screen_clear(0, SCREEN_HEIGHT - 1);


    enable_interrupt();

    while (1) {

    };
    return;
}
