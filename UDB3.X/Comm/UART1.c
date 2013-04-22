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
volatile BYTE UART1txBuff[256];
volatile unsigned int UART1tx_RdPtr = 0;
volatile unsigned int UART1tx_WrPtr = 0;

volatile BYTE UART1rxBuff[256];
volatile unsigned int UART1rx_RdPtr = 0;
volatile unsigned int UART1rx_WrPtr = 0;


// Initialization
void UART1_Init(unsigned long int baud){
    U1MODE = 0b0010000000000000 ; // turn off RX, used to clear errors
	U1STA  = 0b0000010100010000 ;

	// Initialize USART1
	U1MODEbits.UARTEN = 1;       // start w/ uart enabled
	U1MODEbits.USIDL  = 1;       // disable on sleep
	U1MODEbits.ALTIO  = 1;       // use u1a(rx,tx) pins
	U1MODEbits.WAKE   = 1;       // enable wake-up
	U1MODEbits.LPBACK = 0;       // disable loop-back
	U1MODEbits.ABAUD  = 0;       // disable auto-baud
	U1MODEbits.PDSEL  = 0b00;    // 8 bit no parity
	U1MODEbits.STSEL  = 0;       // 1 stop bit

  	U1STAbits.UTXEN   = 1;       // turn on TX for now
  	U1STAbits.URXISEL = 0b00;    // trigger rx interrupt on char
  	U1STAbits.OERR    = 0;       // clear overflow errors

  	IFS0bits.U1RXIF   = 0 ;      // clear the RX interrupt
  	IPC2bits.U1RXIP   = 5 ;      // RX priority high
  	IEC0bits.U1RXIE   = 1 ;      // turn on the RX interrupt
  	IFS0bits.U1TXIF   = 0 ;      // clear the TX interrupt
  	IPC2bits.U1TXIP   = 2 ;      // TX priority lower
	U1STAbits.UTXISEL = 0;      // Interrupt when any space is available in buffer
  	IEC0bits.U1TXIE   = 1 ;      // turn on the TX interrupt

	// Setup the baud rate :
	// Baud Rate = FCY / (16*(BRG+1))
	// Fcy = Fosc/4 = 120MHz/4 = 30MHz
        if( baud == 57600){
            U1BRG = 31; // 57600
        }else if( baud == 115200){
            U1BRG = 15; // 115200
        }else if(baud == 38400){
            U1BRG = 47; // 38400
        }else if(baud == 9600){
            U1BRG = 191;
        }else{
            U1BRG = 47; // Default baud is 38400
        }


}

// Send generic packet
void UART1_SendPacket(BYTE packetId, BYTE len, BYTE* data)
{
    // Disable the UART1 TX interrupt while writing data to buffer
    IEC0bits.U1TXIE   = 0;

    // Build the packet to send
    // Start Checksum
    BYTE chksum = 0;
    BYTE i;

    if (len > 240)
        return;

    // STX
    UART1txBuff[UART1tx_WrPtr++] = STXA;
    UART1txBuff[UART1tx_WrPtr++] = STXB;

    // Paket Id
    UART1txBuff[UART1tx_WrPtr++] = packetId;
    chksum += packetId;

    // Paket Data Length
    UART1txBuff[UART1tx_WrPtr++] = len;
    chksum += len;

    // Stuff Data
    for(i = 0; i < len; i++)
    {
        // Send Byte
        UART1txBuff[UART1tx_WrPtr++] = data[i];
        chksum += data[i];
    }

    // Checksum Calculate and Send
    chksum ^= 0xFF;
    chksum += 1;
    UART1txBuff[UART1tx_WrPtr++] = chksum;

    // Enable the UART1 TX interrupt and fire it
    IEC0bits.U1TXIE   = 1 ;
    IFS0bits.U1TXIF   = 1 ;
}
//======================================================================

// Read and parse any bytes sitting on the RX buffer
// Call this often to prevent buffer overflow
void UART1_FlushRX(void){

}

// UART1 receive interrupt, just put byte onto a buffer that will be serviced later
void __attribute__((__interrupt__,__no_auto_psv__)) _U1RXInterrupt(void)
{
  	// Read character into the buffer and increment pointer
  	UART1rxBuff[UART1rx_WrPtr++] = U1RXREG;

  	// handle serial errors
  	if( U1STAbits.OERR ){   // || U1STAbits.FERR
            U1STAbits.OERR    = 0;
  	}
        
        // clear the interrupt
  	IFS0bits.U1RXIF = 0;
}

// UART1 transmit interrupt, just put next byte onto the buffer
void __attribute__((__interrupt__,__no_auto_psv__)) _U1TXInterrupt(void)
{
	// If the read pointer hasn't caught up to the write pointer, send the next byte
	if( UART1tx_RdPtr != UART1tx_WrPtr ){
            U1TXREG = UART1txBuff[UART1tx_RdPtr++];
	}

        // handle serial errors
  	if( U1STAbits.OERR ){   // || U1STAbits.FERR
            U1STAbits.OERR    = 0;
  	}

        // clear the interrupt
	IFS0bits.U1TXIF = 0;
}