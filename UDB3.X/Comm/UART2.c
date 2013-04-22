#include "Comm.h"
#include "p30f4011.h"
#include "../defines.h"
#include "../AHRS/AHRS.h"

// Globals
extern volatile tSensorCal SensorCal;
extern volatile tSensorData SensorData;
extern volatile tAHRSdata AHRSdata;
extern volatile tCmdData CmdData;
extern volatile tLoopFlags loop;
extern volatile tGains Gains;
extern volatile int16_t vbatt;
extern volatile tRCdata RCdata;
extern _Q16 num512, num2p0;


// Transmit and Receive buffers
volatile BYTE UART2txBuff[256];
volatile unsigned int UART2tx_RdPtr = 0;
volatile unsigned int UART2tx_WrPtr = 0;

volatile BYTE UART2rxBuff[256];
volatile unsigned int UART2rx_RdPtr = 0;
volatile unsigned int UART2rx_WrPtr = 0;


// Initialization
void UART2_Init(unsigned long int baud){
    U2MODE = 0b0010000000000000 ; // turn off RX, used to clear errors
	U2STA  = 0b0000010100010000 ;

	// Initialize USART1
	U2MODEbits.UARTEN = 1;       // start w/ uart enabled
	U2MODEbits.USIDL  = 1;       // disable on sleep
	U2MODEbits.WAKE   = 1;       // enable wake-up
	U2MODEbits.LPBACK = 0;       // disable loop-back
	U2MODEbits.ABAUD  = 0;       // disable auto-baud
	U2MODEbits.PDSEL  = 0b00;    // 8 bit no parity
	U2MODEbits.STSEL  = 0;       // 1 stop bit

  	U2STAbits.UTXEN   = 1;       // turn on TX for now
  	U2STAbits.URXISEL = 0b00;    // trigger rx interrupt on char
  	U2STAbits.OERR    = 0;       // clear overflow errors

  	IFS1bits.U2RXIF   = 0 ;      // clear the RX interrupt
  	IPC6bits.U2RXIP   = 5 ;      // RX priority high
  	IEC1bits.U2RXIE   = 1 ;      // turn on the RX interrupt
  	IFS1bits.U2TXIF   = 0 ;      // clear the TX interrupt
  	IPC6bits.U2TXIP   = 2 ;      // TX priority lower
	U2STAbits.UTXISEL = 0;      // Interrupt when any space is available in buffer
  	IEC1bits.U2TXIE   = 1 ;      // turn on the TX interrupt

	// Setup the baud rate :
	// Baud Rate = FCY / (16*(BRG+1))
	// Fcy = Fosc/4 = 120MHz/4 = 30MHz
        if( baud == 57600){
            U2BRG = 31; // 57600
        }else if( baud == 115200){
            U2BRG = 15; // 115200
        }else if(baud == 38400){
            U2BRG = 47; // 38400
        }else if(baud == 9600){
            U2BRG = 191;
        }else{
            U2BRG = 47; // Default baud is 38400
        }


}

// Send generic packet
void UART2_SendPacket(BYTE packetId, BYTE len, BYTE* data)
{
    // Disable the UART2 TX interrupt while writing data to buffer
    IEC1bits.U2TXIE   = 0;

    // Build the packet to send
    // Start Checksum
    BYTE chksum = 0;
    BYTE i;

    if (len > 240)
        return;

    // STX
    UART2txBuff[UART2tx_WrPtr++] = STXA;
    UART2txBuff[UART2tx_WrPtr++] = STXB;

    // Paket Id
    UART2txBuff[UART2tx_WrPtr++] = packetId;
    chksum += packetId;

    // Paket Data Length
    UART2txBuff[UART2tx_WrPtr++] = len;
    chksum += len;

    // Stuff Data
    for(i = 0; i < len; i++)
    {
        // Send Byte
        UART2txBuff[UART2tx_WrPtr++] = data[i];
        chksum += data[i];
    }

    // Checksum Calculate and Send
    chksum ^= 0xFF;
    chksum += 1;
    UART2txBuff[UART2tx_WrPtr++] = chksum;

    // Enable the UART2 TX interrupt and fire it
    IEC1bits.U2TXIE   = 1 ;
    IFS1bits.U2TXIF   = 1 ;
}
//======================================================================

// Read and parse any bytes sitting on the RX buffer
// Call this often to prevent buffer overflow
void UART2_FlushRX(void){

}

// UART2 receive interrupt, just put byte onto a buffer that will be serviced later
void __attribute__((__interrupt__,__no_auto_psv__)) _U2RXInterrupt(void)
{
  	// Read character into the buffer and increment pointer
  	UART2rxBuff[UART2rx_WrPtr++] = U2RXREG;

  	// handle serial errors
  	if( U2STAbits.OERR ){   // || U2STAbits.FERR
            U2STAbits.OERR    = 0;
  	}

        // clear the interrupt
  	IFS1bits.U2RXIF = 0;
}

// UART2 transmit interrupt, just put next byte onto the buffer
void __attribute__((__interrupt__,__no_auto_psv__)) _U2TXInterrupt(void)
{
	// If the read pointer hasn't caught up to the write pointer, send the next byte
	if( UART2tx_RdPtr != UART2tx_WrPtr ){
            U2TXREG = UART2txBuff[UART2tx_RdPtr++];
	}

        // handle serial errors
  	if( U2STAbits.OERR ){   // || U2STAbits.FERR
            U2STAbits.OERR    = 0;
  	}

        // clear the interrupt
	IFS1bits.U2TXIF = 0;
}