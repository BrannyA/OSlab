#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#define SHELL_BEGIN 15

static void disable_interrupt() {
    uint32_t cp0_status = GET_CP0_STATUS();
    cp0_status &= 0xfffffffe;
    SET_CP0_STATUS(cp0_status);
}

static void enable_interrupt() {
    uint32_t cp0_status = GET_CP0_STATUS();
    cp0_status |= 0x01;
    SET_CP0_STATUS(cp0_status);
}

static char read_uart_ch(void) {
    char ch = 0;
    unsigned char *read_port = (unsigned char *) (0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *) (0xbfe48000 + 0x05);

    while ((*stat_port & 0x01)) {
        ch = *read_port;
    }
    return ch;
}


struct task_info task4_1 = {"send", (uint32_t) & mac_send_task, USER_PROCESS};
struct task_info task4_2 = {"recv", (uint32_t) & mac_recv_task, USER_PROCESS};
struct task_info task4_3 = {"initmac", (uint32_t) & mac_init_task, USER_PROCESS};

struct task_info *test_tasks[19] = {&task4_3, &task4_1, &task4_2};
int num_test_tasks = 3;


unsigned long process_arg[6];

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
        do{
            //read command from UART port
        read:
            disable_interrupt();
            ch = read_uart_ch();
            enable_interrupt();
            if(ch == 8 || ch == 127){
                index --;
                screen_cursor_x--;
                buffer[index] = '\0';
                goto read;
            }
            else{
                printf("%c", ch);
            }
        } while(ch == 0);

        if(ch == 13){
            // printf("buffer: %s", buffer);
            buffer[index] = '\0';
            if(strcmp(buffer, "ps") == 0){
                shell_location ++;
                sys_move_cursor(1, shell_location);
                sys_process_show(&shell_location);
            }
            else if(strcmp(buffer, "clear") == 0){
                sys_clear(SHELL_BEGIN, SCREEN_HEIGHT - 1);
                shell_location = SHELL_BEGIN - 1;
            }
            //exec x\0
            else if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'e' && buffer[3] == 'c' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] == '\0'){
                int num = buffer[5] - '0';
                sys_spawn(test_tasks[num]);
                shell_location ++;
                sys_move_cursor(1, shell_location);
                printf("exec process[%c].", buffer[5]);
            }
            //exec xx\0  xx = 10-15
            else if(buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'e' && buffer[3] == 'c' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] >= '0' && buffer[6] <= '6' && buffer[7] == '\0' ){
                int num = 10 * (buffer[5] - '0') + buffer[6] - '0';
                sys_spawn(test_tasks[num]);
                shell_location ++;
                sys_move_cursor(1, shell_location);
                printf("exec process[%c%c].", buffer[5], buffer[6]);
            }
            //kill x\0
            else if(buffer[0] == 'k' && buffer[1] == 'i' && buffer[2] == 'l' && buffer[3] == 'l' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] == '\0'){
                int i;
                for(i = 0; i < NUM_MAX_TASK; i++){
                    if(pcb[i].pid == buffer[5] - '0' )
                        break;
                }
                if(i == NUM_MAX_TASK){
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("pcb does not exist");
                }
                else if(buffer[5] - '0' == 1){
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("cannot kill shell");
                }
                else{
                    sys_kill(buffer[5] - '0');
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("kill process pid = %c.", buffer[5]);
                }
            }
            //kill xx\0
            else if(buffer[0] == 'k' && buffer[1] == 'i' && buffer[2] == 'l' && buffer[3] == 'l' && buffer[4] ==' ' && buffer[5] >= '0' && buffer[5] <= '9' && buffer[6] >= '0' && buffer[6] <= '6' && buffer[7] == '\0' ){
                int i;
                for(i = 0; i < NUM_MAX_TASK; i++){
                    if(pcb[i].pid == 10 * (buffer[5] - '0') + buffer[6] - '0' )
                        break;
                }
                if(i == NUM_MAX_TASK){
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("pcb does not exist");
                }
                else{
                    sys_kill( 10 * (buffer[5] - '0') + buffer[6] - '0' );
                    shell_location ++;
                    sys_move_cursor(1, shell_location);
                    printf("kill process pid = %c%c.", buffer[5], buffer[6]);
                }
            }
            else{
                shell_location ++;
                sys_move_cursor(1, shell_location);
                printf("Error, try again");
            }
            index = 0;
            shell_location ++;
            sys_move_cursor(1, shell_location);
            printf("> root@UCAS_OS:");
        }
        else{
            buffer[index++] = ch;
        }
    }
}
