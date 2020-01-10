#include "mac.h"
#include "irq.h"
#include "type.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#include "test4.h"

desc_t *send_desc;
desc_t *receive_desc;
uint32_t cnt = 1; //record the time of iqr_mac
uint32_t buffer[PSIZE] = {0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
                          0x00e00101, 0xe914fb00, 0x0004e914, 0x0000, 0x005e0001, 0x2300fb00, 0x84b7f28b, 0x00450008,
                          0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};

static void mac_send_desc_init(mac_t *mac) {
    int i, j;
    uint32_t TxBuff = TXBUFFADDR;
    bzero((uint8_t *) TxBuff, PNUM * BUFFSIZE);
    for (i = 0; i < 63; i++) {
        Tx_desc[i].tdes0 = 0;
        Tx_desc[i].tdes1 = DescTxLast | DescTxFirst | TxDescChain | PSIZE * 4;
        Tx_desc[i].tdes2 = TxBuff & 0x1fffffff;// Physical Address
        Tx_desc[i].tdes3 = ((int) &Tx_desc[i + 1]) & 0x1fffffff;   // Physical Address
        memcpy((uint8_t *) TxBuff, buffer, sizeof(buffer));

        TxBuff += BUFFSIZE;
    }

    Tx_desc[63].tdes0 = 0;
    Tx_desc[63].tdes1 = DescTxLast | DescTxFirst | TxDescEndOfRing | TxDescChain | PSIZE * 4;
    Tx_desc[63].tdes2 = TxBuff & 0x1fffffff;
    Tx_desc[63].tdes3 = ((int) &Tx_desc[0]) & 0x1fffffff;
    memcpy((uint8_t *) TxBuff, buffer, sizeof(buffer));

    mac->saddr = TxBuff;
    mac->saddr_phy = TxBuff & 0x1fffffff;

    mac->td = (uint32_t) & Tx_desc[0];
    mac->td_phy = ((uint32_t) & Tx_desc[0]) & 0x1fffffff;
}

static void mac_recv_desc_init(mac_t *mac) {
    int i, j;
    uint32_t RxBuff = RXBUFFADDR;
    bzero((uint8_t *) RxBuff, PNUM * BUFFSIZE);


    for (i = 0; i < PNUM; i++) {
        Rx_desc[i].tdes0 = 0;
        Rx_desc[i].tdes1 = 0x81000400;
        Rx_desc[i].tdes2 = RxBuff & 0x1fffffff;   // Physical Address
        Rx_desc[i].tdes3 = ((int) &Rx_desc[i + 1]) & 0x1fffffff;   // Physical Address
    }

    Rx_desc[PNUM - 1].tdes0 = 0;
    Rx_desc[PNUM - 1].tdes1 = 0x03000400;
    Rx_desc[PNUM - 1].tdes2 = RxBuff & 0x1fffffff;
    Rx_desc[PNUM - 1].tdes3 = ((int) &Rx_desc[0]) & 0x1fffffff;

    mac->daddr = RxBuff;
    mac->daddr_phy = RxBuff & 0x1fffffff;

    mac->rd = (uint32_t) & Rx_desc[0];
    mac->rd_phy = ((uint32_t) & Rx_desc[0]) & 0x1fffffff;
}

static void mii_dul_force(mac_t *mac) {
    reg_write_32(mac->dma_addr, 0x80);
    //   reg_write_32(mac->dma_addr, 0x400);
    uint32_t conf = 0xc800;            //0x0080cc00;

    // loopback, 100M
    reg_write_32(mac->mac_addr, reg_read_32(mac->mac_addr) | (conf) | (1 << 8));
    //enable recieve all
    reg_write_32(mac->mac_addr + 0x4, reg_read_32(mac->mac_addr + 0x4) | 0x80000001);
}

void dma_control_init(mac_t *mac, uint32_t init_value) {
    reg_write_32(mac->dma_addr + DmaControl, init_value);
    return;
}

void mac_send_task() {
    mac_t test_mac;
    uint32_t i;
    uint32_t print_location = 2;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum

    mac_send_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt(&test_mac);
    mii_dul_force(&test_mac);

    irq_enable(LS1C_MAC_IRQ);
    sys_move_cursor(1, print_location);
    printf("> [MAC SEND TASK] start send package.               \n");

    uint32_t cnt = 0;
    i = 4;
    while (i > 0) {
        sys_net_send(test_mac.td, test_mac.td_phy);
        cnt += PNUM;
        sys_move_cursor(1, print_location);
        printf("> [MAC SEND TASK] totally send package %d !        \n", cnt);
        i--;
    }
    sys_exit();
}

void mac_recv_task() {
    // static int time = 0;
    while(1 ){
        int clr_flg;
        for(clr_flg = 0; clr_flg < PNUM; clr_flg++){
            recv_flag[clr_flg] = 0;
        }
        received_num = 0;
        sys_init_mac();
        mac_t test_mac;
        uint32_t i;
        uint32_t ret;
        uint32_t print_location = 1;

        test_mac.mac_addr = 0xbfe10000;
        test_mac.dma_addr = 0xbfe11000;

        test_mac.psize = PSIZE * 4; // 64bytes
        test_mac.pnum = PNUM;       // pnum
        // if(time == 1)
        //     print_location = 8;
        // time ++;
        mac_recv_desc_init(&test_mac);

        dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
        clear_interrupt();

        mii_dul_force(&test_mac);

        irq_enable(LS1C_MAC_IRQ);
        queue_init(&recv_block_queue);
        sys_move_cursor(1, print_location);
        printf("[RECV TASK] start recv:                    ");
        ret = sys_net_recv(test_mac.rd, test_mac.rd_phy, test_mac.daddr);
        // ch_flag = 0;
        for (i = 0; i < PNUM; i++) {
            recv_flag[i] = (uint32_t)(test_mac.rd + i * 16);
        }
        recv_flag[0] = (uint32_t) test_mac.rd;
        uint32_t cnt = 0;
        uint32_t *Recv_desc;
        Recv_desc = (uint32_t * )(test_mac.rd + (PNUM - 1) * 16);
        if (((*Recv_desc) & 0x80000000) == 0x80000000) {
            sys_move_cursor(1, print_location);
            printf("> [RECV TASK] waiting receive package.\n");
            sys_wait_recv_package();
        }
        mac_recv_handle(&test_mac);
    }
    sys_exit();
}

void mac_init_task() {
    uint32_t print_location = 1;
    sys_move_cursor(1, print_location);
    printf("> [MAC INIT TASK] Waiting for MAC initialization .\n");
    sys_init_mac();
    sys_move_cursor(1, print_location);
    printf("> [MAC INIT TASK] MAC initialization succeeded.           \n");
    sys_exit();
}
