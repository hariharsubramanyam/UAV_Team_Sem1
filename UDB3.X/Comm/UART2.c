#include "Comm.h"
#include "p30F4011.h"
#include "../defines.h"
#include "../AHRS/AHRS.h"

// Globals
extern volatile tSensorCal SensorCal;
extern volatile tSensorData SensorData;
extern volatile tAHRSdata AHRSdata;
extern volatile tCmdData CmdData;
extern volatile tLoopFlags loop;
extern volatile tGains Gains;
extern volatile tRCdata RCdata;
extern _Q16 num512,num1024, num2p0;


// Transmit and Receive buffers
#define UART2_TXBUFFSIZE 128
volatile BYTE UART2txBuff[UART2_TXBUFFSIZE];
volatile unsigned int UART2tx_RdPtr = 0;
volatile unsigned int UART2tx_WrPtr = 0;

#define UART2_RXBUFFSIZE 128
volatile BYTE UART2rxBuff[UART2_RXBUFFSIZE];
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

volatile BYTE spektrumData[14];
void UART2_ProcessSpektrumData( )
{
    // PROCESS Spektrum receiver data -- see http://www.desertrc.com/spektrum_protocol.htm for protocol
    // Determine what to do with received character
    
    uint8_t i;
    int16_t scaleInt;
    _Q16 tmpQ16, scaleQ16;
    // Data is 14 bytes long
    for( i=0; i<14; i+=2)
    {

        uint16_t rcdata = spektrumData[i] << 8; // MSB
        rcdata += spektrumData[i+1];	// LSB
#ifdef DSMX
        // 11-bit data mode
        uint16_t cmddata = (int16_t) (rcdata & 0b0000011111111111); // get last 11 bits
        uint8_t channel = rcdata >> 11; // get 5 first bits
        scaleQ16 = num1024;
        scaleInt = 8;
#else
        // 10-bit data mode
        uint16_t cmddata = (int16_t) (rcdata & 0b0000001111111111); // get last 10 bits
        uint8_t channel = rcdata >> 10; // get 6 first bits
        scaleQ16 = num512;
        scaleInt = 4;
#endif


        switch(channel){	// process channel data
            case 0:
                RCdata.ch0 = cmddata;
                break;
            case 1:
                int16toQ16(&tmpQ16,&cmddata);
                tmpQ16 -= scaleQ16;
                RCdata.ch1 = _IQ16div(tmpQ16,scaleQ16);
                break;
            case 2:
                int16toQ16(&tmpQ16,&cmddata);
                tmpQ16 -= scaleQ16;
                RCdata.ch2 = _IQ16div(tmpQ16,scaleQ16);
                break;
            case 3:
                int16toQ16(&tmpQ16,&cmddata);
                tmpQ16 -= scaleQ16;
                RCdata.ch3 = _IQ16div(tmpQ16,scaleQ16);
                break;
            case 4:
                RCdata.ch4 = cmddata;
                break;
            case 5:
                RCdata.ch5 = cmddata;
                break;
            case 6:
                RCdata.ch6 = cmddata;
                break;
            default:
                break;
        } // End switch channel
    } // End for each data byte

    // manual mode
    if (RCdata.ch4 > 600) {
        CmdData.AttCmd = 1;
        CmdData.throttle = RCdata.ch0/scaleInt;  // RCdata is between 0 and 1023, this will be approximately between 0 and 255
        _Q16 halfRoll = -RCdata.ch1; //mult(RCdata.ch1,num0p5);  // this is about -0.6 -> 0.6  which is +/- ~70 degrees
        _Q16 halfPitch = RCdata.ch2; //mult(RCdata.ch2,num0p5);
        _Q16 cosRoll = _Q16cos(halfRoll);
        _Q16 sinRoll = _Q16sin(halfRoll);
        _Q16 cosPitch = _Q16cos(halfPitch);
        _Q16 sinPitch = _Q16sin(halfPitch);
        CmdData.q_cmd.o = mult(cosRoll,cosPitch);
        CmdData.q_cmd.x = mult(sinRoll,cosPitch);
        CmdData.q_cmd.y = mult(cosRoll,sinPitch);
        CmdData.q_cmd.z = -mult(sinRoll,sinPitch);
        CmdData.p = CmdData.q = 0;
        CmdData.r = mult(RCdata.ch3,num2p0);

        // Turn on green LED to signify manual control mode ON
        led_on(LED_GREEN);
    } else
    {
        // Turn off green LED to signify manual control mode OFF
        led_off(LED_GREEN);
        CmdData.AttCmd = 0;
    }

    // Switch on ch5
    if ((RCdata.ch5 > 600) || (RCdata.ch6 > 600)) {
        //CmdData.Switch = 1;
        //led_on(LED_RED);
    } else
    {
        //CmdData.Switch = 0;
        //led_off(LED_RED);
    }

}

volatile uint16_t msSinceSpektrumByte = 0;

// UART2 receive interrupt, just put byte onto a buffer that will be serviced later
void __attribute__((__interrupt__,__no_auto_psv__)) _U2RXInterrupt(void)
{
    // Check if this is the start of a new data frame
    if(msSinceSpektrumByte > 7){

        // Make sure packet size is correct
        if(UART2rx_WrPtr == 16){
            // Copy data, discarding first two bytes
            memcpy(spektrumData, UART2rxBuff+2, 14);
            // Signal spektrum data processing
            loop.ProcessSpektrum = 1;
        }

        UART2rx_RdPtr = 0;
        UART2rx_WrPtr = 0;
    }

    msSinceSpektrumByte = 0;

    // Read character into the buffer and increment pointer
    if(UART2rx_WrPtr < UART2_RXBUFFSIZE){
        UART2rxBuff[UART2rx_WrPtr++] = U2RXREG;
    }
    
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
    // clear the interrupt
    IFS1bits.U2TXIF = 0;

    // If the read pointer hasn't caught up to the write pointer, send the next byte
    if( UART2tx_RdPtr < UART2tx_WrPtr ){
        U2TXREG = UART2txBuff[UART2tx_RdPtr++];
    }

    // If the full buffer has been sent, reset
    if(UART2tx_RdPtr >= UART2tx_WrPtr){
        UART2tx_WrPtr = 0;
        UART2tx_RdPtr = 0;
    }

    // handle serial errors
    if( U2STAbits.OERR  || U2STAbits.FERR || U2STAbits.PERR)
    {
        U2STAbits.OERR    = 0;
        U2STAbits.FERR = 0;
        U2STAbits.PERR = 0;
    }

}