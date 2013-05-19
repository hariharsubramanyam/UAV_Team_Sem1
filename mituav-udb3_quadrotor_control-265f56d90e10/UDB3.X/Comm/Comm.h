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

#define PACKETID_AHRS 0x01
typedef struct sAHRSPacket {
    int16_t qw;
    int16_t qx;
    int16_t qy;
    int16_t qz;
    int16_t p;
    int16_t q;
    int16_t r;
} tAHRSPacket;


#define PACKETID_AHRS_EXT 0x02
typedef struct sAHRSEXTPacket {
    int16_t qw;
    int16_t qx;
    int16_t qy;
    int16_t qz;
    int16_t qcw;
    int16_t qcx;
    int16_t qcy;
    int16_t qcz;
    int16_t p;
    int16_t q;
    int16_t r;
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t intPitch;
    int16_t intRoll;
    int16_t intYaw;
} tAHRSEXTPacket;

// Prototypes
void UART1_SendPacket(BYTE packetId, BYTE len, BYTE* data);
void UART1_FlushRX(void);
void UART1_Init(unsigned long int baud);
void UART1_SendAHRSpacket();

void UART2_SendPacket(BYTE packetId, BYTE len, BYTE* data);
void UART2_ProcessSpektrumData( );
void UART2_Init(unsigned long int baud);

void UART_ParsePacket(struct strPacket * rxPacket);

// Defines
#define STXA 0xFF
#define STXB 0xFE
#define RCXA 0xFF
#define RCXB 0xA2

#define RXSTATE_STXA			0
#define RXSTATE_STXB			1
#define RXSTATE_PACKETID		2
#define RXSTATE_LEN			3
#define RXSTATE_DATA			4
#define RXSTATE_CHKSUM			5

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$



#endif