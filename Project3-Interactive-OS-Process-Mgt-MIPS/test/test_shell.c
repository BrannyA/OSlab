/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
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
#include "lock.h"
#include "sched.h"
#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"

static void disable_interrupt()
{
    uint32_t cp0_status = GET_CP0_STATUS();
    cp0_status &= 0xfffffffe;
    SET_CP0_STATUS(cp0_status);
}

static void enable_interrupt()
{
    uint32_t cp0_status = GET_CP0_STATUS();
    cp0_status |= 0x01;
    SET_CP0_STATUS(cp0_status);
}

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}
struct task_info task1 = {"task1", (uint32_t)&ready_to_exit_task, USER_PROCESS};
struct task_info task2 = {"task2", (uint32_t)&wait_lock_task, USER_PROCESS};
struct task_info task3 = {"task3", (uint32_t)&wait_exit_task, USER_PROCESS};

struct task_info task4 = {"task4", (uint32_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task5 = {"task5", (uint32_t)&semaphore_add_task2, USER_PROCESS};
struct task_info task6 = {"task6", (uint32_t)&semaphore_add_task3, USER_PROCESS};

struct task_info task7 = {"task7", (uint32_t)&producer_task, USER_PROCESS};
struct task_info task8 = {"task8", (uint32_t)&consumer_task1, USER_PROCESS};
struct task_info task9 = {"task9", (uint32_t)&consumer_task2, USER_PROCESS};

struct task_info task10 = {"task10", (uint32_t)&barrier_task1, USER_PROCESS};
struct task_info task11 = {"task11", (uint32_t)&barrier_task2, USER_PROCESS};
struct task_info task12 = {"task12", (uint32_t)&barrier_task3, USER_PROCESS};

struct task_info task13 = {"SunQuan",(uint32_t)&SunQuan, USER_PROCESS};
struct task_info task14 = {"LiuBei", (uint32_t)&LiuBei, USER_PROCESS};
struct task_info task15 = {"CaoCao", (uint32_t)&CaoCao, USER_PROCESS};

struct task_info *test_tasks[16] = {&task1, &task2, &task3,
                                           &task4, &task5, &task6,
                                           &task7, &task8, &task9,
                                           &task10, &task11, &task12,
                                           &task13, &task14, &task15};
int num_test_tasks = 15;

void test_shell()
{
    // printk("shell");
    char ch = 0;
    char buffer[16];
    int index = 0;
    int shell_location = SHELL_BEGIN;

    sys_move_cursor(1, SHELL_BEGIN-1);
    printf("------------------- COMMAND -------------------");
    sys_move_cursor(1, SHELL_BEGIN);
    printf("> root@UCAS_OS:");

    while(1)
    {
        do{
            //read command from UART port
        read:
            disable_interrupt();
            ch = read_uart_ch();
            enable_interrupt();
            if(ch == 8 || ch == 127)
            {
                index --;
                screen_cursor_x--;
                buffer[index] = '\0';
                goto read;
            }
            else
            {
                printf("%c", ch);
            }
        } while(ch == 0);

        if(ch == 13)    // \n
        {
            // printf("buffer: %s", buffer);
            buffer[index] = '\0';
            if(strcmp(buffer, "ps") == 0)
            {
                shell_location ++;
                sys_move_cursor(1, shell_location);
                sys_ps(&shell_location);
            }
            else if(strcmp(buffer, "clear") == 0)
            {
                sys_clear(SHELL_BEGIN, SCREEN_HEIGHT - 1);
                shell_location = SHELL_BEGIN - 1;
            }
            //exec x\0
            else if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'e' && buffer[3] == 'c' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] == '\0')
            {
                int num = buffer[5] - '0';
                sys_spawn(test_tasks[num]);
                shell_location ++;
                sys_move_cursor(1, shell_location);
                printf("exec process[%c].", buffer[5]);
            }
            //exec xx\0  xx = 10-15
            else if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'e' && buffer[3] == 'c' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] >= '0' && buffer[6] <= '6' && buffer[7] == '\0' )
            {
                int num = 10 * (buffer[5] - '0') + buffer[6] - '0';
                sys_spawn(test_tasks[num]);
                shell_location ++;
                sys_move_cursor(1, shell_location);
                printf("exec process[%c%c].", buffer[5], buffer[6]);
            }
            //kill x\0
            else if(buffer[0] == 'k' && buffer[1] == 'i' && buffer[2] == 'l' && buffer[3] == 'l' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] == '\0')
            {
                int i;
                for(i = 0; i < NUM_MAX_TASK; i++)
                {
                    if(pcb[i].pid == buffer[5] - '0' )
                        break;
                }
                if(i == NUM_MAX_TASK)
                {
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("pcb does not exist");
                }
                else if(buffer[5] - '0' == 1)
                {
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("cannot kill shell");
                }
                else
                {
                    sys_kill(buffer[5] - '0');
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("kill process pid = %c.", buffer[5]);
                }
            }
            //kill xx\0
            else if(buffer[0] == 'k' && buffer[1] == 'i' && buffer[2] == 'l' && buffer[3] == 'l' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] >= '0' && buffer[6] <= '6' && buffer[7] == '\0' )
            {
                int i;
                for(i = 0; i < NUM_MAX_TASK; i++)
                {
                    if(pcb[i].pid == 10 * (buffer[5] - '0') + buffer[6] - '0' )
                        break;
                }
                if(i == NUM_MAX_TASK)
                {
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("pcb does not exist");
                }
                else
                {
                    sys_kill( 10 * (buffer[5] - '0') + buffer[6] - '0' );
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("kill process pid = %c%c.", buffer[5], buffer[6]);
                }
            }
            else
            {
                shell_location ++;
                sys_move_cursor(1, shell_location);
                printf("Error, try again");
            }
            index = 0;
            shell_location ++;
            sys_move_cursor(1, shell_location);
            printf("> root@UCAS_OS:");
        }
        else
        {
            buffer[index++] = ch;
        }
    }
}

struct task_info task_shell = {"test_shell", (uint32_t)&test_shell, USER_PROCESS, 1};
