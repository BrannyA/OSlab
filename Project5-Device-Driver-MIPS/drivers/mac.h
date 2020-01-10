#ifndef INCLUDE_MAC_H_
#define INCLUDE_MAC_H_

#include "type.h"
#include "queue.h"
//#define TEST_REGS1
//#define TEST_REGS2
#define TEST_REGS3
#define GMAC_BASE_ADDR (0xbfe10000)
#define DMA_BASE_ADDR (0xbfe11000)
#define PSIZE (256)
#define PNUM (64)

#define LS1C_MAC_IRQ (0x8)
#define INT1_CLR  0xbfd01064
#define INT1_POL  0xbfd01068
#define INT1_EDGE 0xbfd0106c
#define TXBUFFADDR (0xa0f00400)
#define RXBUFFADDR (0xa0f10400)
#define BUFFSIZE 1024
extern int received_num;
extern uint32_t cnt;
typedef struct desc {
    uint32_t tdes0;
    uint32_t tdes1;
    uint32_t tdes2;
    uint32_t tdes3;
} desc_t;


typedef struct mac {
    uint32_t psize; // backpack size
    uint32_t pnum;
    uint32_t mac_addr; // MAC base address
    uint32_t dma_addr; // DMA base address

    uint32_t saddr; // send address
    uint32_t daddr; // receive address

    uint32_t saddr_phy; // send phy address
    uint32_t daddr_phy; // receive phy address

    uint32_t td; // DMA send desc
    uint32_t rd; // DMA receive desc

    uint32_t td_phy;
    uint32_t rd_phy;

} mac_t;

extern desc_t Tx_desc[256];  // Send
extern desc_t Rx_desc[256];  // Receive

extern uint32_t Tx_buffer[256][PSIZE];  // Send
extern uint32_t Rx_buffer[256][PSIZE];  // Receive

extern queue_t recv_block_queue;
extern uint32_t recv_flag[256];
extern uint32_t ch_flag;
enum GmacRegisters {
    GmacAddr0Low = 0x0044,  /* Mac frame filtering controls */
    GmacAddr0High = 0x0040, /* Mac address0 high Register  */
};

enum DmaRegisters {
    DmaBusMode = 0x0000,      /* CSR0 - Bus Mode Register                          */
    DmaTxPollDemand = 0x0004, /* CSR1 - Transmit Poll Demand Register              */
    DmaRxPollDemand = 0x0008, /* CSR2 - Receive Poll Demand Register               */
    DmaRxBaseAddr = 0x000C,   /* CSR3 - Receive Descriptor list base address       */
    DmaTxBaseAddr = 0x0010,   /* CSR4 - Transmit Descriptor list base address      */
    DmaStatus = 0x0014,       /* CSR5 - Dma status Register                        */
    DmaControl = 0x0018,      /* CSR6 - Dma Operation Mode Register                */
    DmaInterrupt = 0x001C,    /* CSR7 - Interrupt enable                           */
    DmaMissedFr = 0x0020,     /* CSR8 - Missed Frame & Buffer overflow Counter     */
    DmaTxCurrDesc = 0x0048,   /* CSR18 - Current host Tx Desc Register              */
    DmaRxCurrDesc = 0x004C,   /* CSR19 - Current host Rx Desc Register              */
    DmaTxCurrAddr = 0x0050,   /* CSR20 - Current host transmit buffer address      */
    DmaRxCurrAddr = 0x0054,   /* CSR21 - Current host receive buffer address       */
};

enum DmaDescriptorStatus /* status word of DMA descriptor */
{
    DescOwnByDma = 0x80000000, /* (OWN)Descriptor is owned by DMA engine            31      RW                  */

    DescDAFilterFail = 0x40000000, /* (AFM)Rx - DA Filter Fail for the rx frame         30                          */

    DescFrameLengthMask = 0x3FFF0000, /* (FL)Receive descriptor frame length               29:16                       */
    DescFrameLengthShift = 16,

    DescError = 0x00008000,         /* (ES)Error summary bit  - OR of the follo. bits:   15                          */
    /*  DE || OE || IPC || LC || RWT || RE || CE */
    DescRxTruncated = 0x00004000,   /* (DE)Rx - no more descriptors for receive frame    14                          */
    DescSAFilterFail = 0x00002000,  /* (SAF)Rx - SA Filter Fail for the received frame   13                          */
    DescRxLengthError = 0x00001000, /* (LE)Rx - frm size not matching with len field     12                          */
    DescRxDamaged = 0x00000800,     /* (OE)Rx - frm was damaged due to buffer overflow   11                          */
    DescRxVLANTag = 0x00000400,     /* (VLAN)Rx - received frame is a VLAN frame         10                          */
    DescRxFirst = 0x00000200,       /* (FS)Rx - first descriptor of the frame             9                          */
    DescRxLast = 0x00000100,        /* (LS)Rx - last descriptor of the frame              8                          */
    DescRxLongFrame = 0x00000080,   /* (Giant Frame)Rx - frame is longer than 1518/1522   7                          */
    DescRxCollision = 0x00000040,   /* (LC)Rx - late collision occurred during reception  6                          */
    DescRxFrameEther = 0x00000020,  /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5                          */
    DescRxWatchdog = 0x00000010,    /* (RWT)Rx - watchdog timer expired during reception  4                          */
    DescRxMiiError = 0x00000008,    /* (RE)Rx - error reported by MII interface           3                          */
    DescRxDribbling = 0x00000004,   /* (DE)Rx - frame contains non int multiple of 8 bits 2                          */
    DescRxCrc = 0x00000002,         /* (CE)Rx - CRC error                                 1                          */
    DescRxMacMatch = 0x00000001,    /* (RX MAC Address) Rx mac address reg(1 to 15)match  0                          */

    //Rx Descriptor Checksum Offload engine (type 2) encoding
    //DescRxPayChkError     = 0x00000001,   /* ()  Rx - Rx Payload Checksum Error                 0                          */
    //DescRxIpv4ChkError    = 0x00000080,   /* (IPC CS ERROR)Rx - Ipv4 header checksum error      7                          */

    DescRxChkBit0 = 0x00000001, /*()  Rx - Rx Payload Checksum Error                  0                          */
    DescRxChkBit7 = 0x00000080, /* (IPC CS ERROR)Rx - Ipv4 header checksum error      7                          */
    DescRxChkBit5 = 0x00000020, /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5                          */

    DescTxIpv4ChkError = 0x00010000,  /* (IHE) Tx Ip header error                           16                         */
    DescTxTimeout = 0x00004000,       /* (JT)Tx - Transmit jabber timeout                   14                         */
    DescTxFrameFlushed = 0x00002000,  /* (FF)Tx - DMA/MTL flushed the frame due to SW flush 13                         */
    DescTxPayChkError = 0x00001000,   /* (PCE) Tx Payload checksum Error                    12                         */
    DescTxLostCarrier = 0x00000800,   /* (LC)Tx - carrier lost during tramsmission          11                         */
    DescTxNoCarrier = 0x00000400,     /* (NC)Tx - no carrier signal from the tranceiver     10                         */
    DescTxLateCollision = 0x00000200, /* (LC)Tx - transmission aborted due to collision      9                         */
    DescTxExcCollisions = 0x00000100, /* (EC)Tx - transmission aborted after 16 collisions   8                         */
    DescTxVLANFrame = 0x00000080,     /* (VF)Tx - VLAN-type frame                            7                         */

    DescTxCollMask = 0x00000078, /* (CC)Tx - Collision count                           6:3                        */
    DescTxCollShift = 3,

    DescTxExcDeferral = 0x00000004, /* (ED)Tx - excessive deferral                          2                        */
    DescTxUnderflow = 0x00000002,   /* (UF)Tx - late data arrival from the memory           1                        */
    DescTxDeferred = 0x00000001,    /* (DB)Tx - frame transmision deferred                  0                        */

    /*
	This explains the RDES1/TDES1 bits layout
			  --------------------------------------------------------------------
	    RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1          |
			  --------------------------------------------------------------------

	*/
    //DmaDescriptorLength     length word of DMA descriptor

    DescTxIntEnable = 0x80000000,  /* (IC)Tx - interrupt on completion                    31                       */
    DescTxLast = 0x40000000,       /* (LS)Tx - Last segment of the frame                  30                       */
    DescTxFirst = 0x20000000,      /* (FS)Tx - First segment of the frame                 29                       */
    DescTxDisableCrc = 0x04000000, /* (DC)Tx - Add CRC disabled (first segment only)      26                       */

    RxDisIntCompl = 0x80000000,   /* (Disable Rx int on completion) 			31			*/
    RxDescEndOfRing = 0x02000000, /* (TER)End of descriptors ring                                                 */
    RxDescChain = 0x01000000,     /* (TCH)Second buffer address is chain address         24                       */

    DescTxDisablePadd = 0x00800000, /* (DP)disable padding, added by - reyaz               23                       */

    TxDescEndOfRing = 0x02000000, /* (TER)End of descriptors ring                                                 */
    TxDescChain = 0x01000000,     /* (TCH)Second buffer address is chain address         24                       */

    DescSize2Mask = 0x003FF800, /* (TBS2) Buffer 2 size                                21:11                    */
    DescSize2Shift = 11,
    DescSize1Mask = 0x000007FF, /* (TBS1) Buffer 1 size                                10:0                     */
    DescSize1Shift = 0,

    DescTxCisMask = 0x18000000,        /* Tx checksum offloading control mask			28:27			*/
    DescTxCisBypass = 0x00000000,      /* Checksum bypass								*/
    DescTxCisIpv4HdrCs = 0x08000000,   /* IPv4 header checksum								*/
    DescTxCisTcpOnlyCs = 0x10000000,   /* TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present	*/
    DescTxCisTcpPseudoCs = 0x18000000, /* TCP/UDP/ICMP checksum fully in hardware including pseudo header		*/
};


enum DmaControlReg {
    DmaStoreAndForward = 0x00200000, /* (SF)Store and forward                            21      RW        0       */
    DmaRxThreshCtrl128 = 0x00000018, /* (RTC)Controls thre Threh of MTL tx Fifo 128      4:3   RW                */
    DmaTxStart = 0x00002000, /* (ST)Start/Stop transmission                      13      RW        0       */
    DmaTxSecondFrame = 0x00000004, /* (OSF)Operate on second frame                     4       RW        0       */
};

enum InitialRegisters {
    DmaIntDisable = 0,
};

uint32_t read_register(uint32_t base, uint32_t offset);

void reg_write_32(uint32_t addr, uint32_t data);

void print_rx_dscrb(mac_t *mac);

void print_tx_dscrb(mac_t *mac);

uint32_t do_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr);

void do_net_send(uint32_t td, uint32_t td_phy);

void do_init_mac(void);

void do_wait_recv_package(void);

void mac_irq_handle(void);

void mac_recv_handle(mac_t *test_mac);

void clear_interrupt();

int test_mac_recv_glb;

#endif
