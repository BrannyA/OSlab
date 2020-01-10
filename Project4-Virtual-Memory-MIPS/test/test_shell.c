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

#include "mm.h"
#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"
#include "test4.h"

unsigned int a[6];

int shell_atoi(char *x)
{
    if (x >= '0' && x <= '9')
    {
        return x - '0';
    }
    else if (x >= 'A' && x <= 'F')
    {
        return x - 'A' + 10;
    }
    else return 0;
}

static void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *) (0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *) (0xbfe48000 + 0x05);

    while ((*stat_port & 0x01)) {
        ch = *read_port;
    }
    return ch;
}

struct task_info task1 = {"task1", (uint32_t) & drawing_task1, USER_PROCESS};
struct task_info task2 = {"task2", (uint32_t) & rw_task1, USER_PROCESS};

struct task_info *test_tasks[2] = {&task1, &task2};

int num_test_tasks = 2;

void test_shell() {
    char ch = 0;
    char buffer[128];
    int index = 0;
    int shell_location = SHELL_BEGIN;

    sys_move_cursor(1, SHELL_BEGIN - 1);
    printf("------------------- COMMAND -------------------");
    sys_move_cursor(1, SHELL_BEGIN);
    printf("> root@UCAS_OS:");

    while (1) {
        do {
    read:
            //read command from UART port
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
        } while (ch == 0);

        if (ch == 13)
        {
            buffer[index] = '\0';
            if (strcmp(buffer, "ps") == 0)
            {
                shell_location++;
                sys_move_cursor(1, shell_location);
                sys_ps(&shell_location);
            }
            else if (strcmp(buffer, "clear") == 0)
            {
                sys_clear(SHELL_BEGIN, SCREEN_HEIGHT - 1);
                shell_location = SHELL_BEGIN - 1;
            }
            else if (buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'e' && buffer[3] == 'c' &&
                       buffer[4] == ' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] == '\0' &&
                       buffer[5] != '1')
            {
                //process single-digit exec number if it is not 1
                int num = buffer[5] - '0';
                sys_spawn(test_tasks[num]);
                shell_location++;
                sys_move_cursor(1, shell_location);
                printf("exec process[%c].", buffer[5]);
            }
            else if (buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'e' && buffer[3] == 'c' &&
                       buffer[4] == ' ' && buffer[5] == '1')
            {
                // further process process 1
                int i, j, add;
                for (i = 0; i < 6; ++i)
                {
                    a[i] = 0;
                    for (j = 0; j < 8; ++j)
                    {
                        a[i] = 16 * a[i] + shell_atoi(buffer[9 + 11 * i + j]);
                    }
                }
                sys_spawn(test_tasks[1]);
                shell_location++;
                sys_move_cursor(1, shell_location);
                printf("exec process[%c].", buffer[5]);
            }
            else if (buffer[0] == 'k' && buffer[1] == 'i' && buffer[2] == 'l' && buffer[3] == 'l' &&
                       buffer[4] == ' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] == '\0') {
                sys_kill(buffer[5] - '0');
                shell_location++;
                sys_move_cursor(1, shell_location);
                printf("kill process pid = %c.", buffer[5]);
            }
            else
            {
                shell_location++;
                sys_move_cursor(1, shell_location);
                printf("Error: Unknown command");
            }
            index = 0;
            shell_location++;
            sys_move_cursor(1, shell_location);
            printf("> root@UCAS_OS:");
        }
        else
            buffer[index++] = ch;
    }
}

struct task_info task_shell = {"shell", (uint32_t) & test_shell, USER_PROCESS};
