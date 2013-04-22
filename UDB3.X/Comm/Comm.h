#ifndef _COMM_H
#define _COMM_H

#include "../defines.h"
#include "../AHRS/AHRS.h"

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// UART1 and UART2
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

// Packet definitions

struct strPacket {
    BYTE id;
    BYTE len;
    BYTE data[256];
};

#define PACKETID_DEBUG 0x01
typedef struct sDebugPacket {
    int16_t val[8];
} tDebugPacket;


// Prototypes
void UART1_SendPacket(BYTE packetId, BYTE len, BYTE* data);
void UART1_FlushRX(void);
void UART1_Init(unsigned long int baud);

void UART2_SendPacket(BYTE packetId, BYTE len, BYTE* data);
void UART2_FlushRX(void);
void UART2_Init(unsigned long int baud);

void UART_ParsePacket(struct strPacket * rxPacket);

// Defines
#define STXA 0xFF
#define STXB 0xFE
#define RCXA 0x03
#define RCXB 0x12

#define RXSTATE_STXA			0
#define RXSTATE_STXB			1
#define RXSTATE_PACKETID		2
#define RXSTATE_LEN			3
#define RXSTATE_DATA			4
#define RXSTATE_CHKSUM			5

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$



#endif