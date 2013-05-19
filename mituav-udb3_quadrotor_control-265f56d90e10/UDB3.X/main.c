/*******************************
 * Written by Buddy Michini and Mark Cutler
 * acl.mit.edu
 *******************************/

#include "defines.h"
#include "Comm/Comm.h"

/******************************* 
Set device configuration values 
********************************/
_FOSC( CSW_FSCM_OFF & FRC_PLL16 );   // Fast RC 7.37MHz internal osc w/ 16x PLL
_FWDT( WDT_OFF );             // no watchdog timer
_FBORPOR( PBOR_OFF &          // brown out detection off
	  MCLR_EN &               // enable MCLR
	  RST_PWMPIN &            // pwm pins as pwm
	  PWMxH_ACT_HI &          // PWMH is active high
	  PWMxL_ACT_HI );         // PMWL is active high
_FGS( CODE_PROT_OFF );        // no protection
_FICD( 0xC003 );	          // normal use of debugging port


/* globals */
extern volatile unsigned long timerCount;
extern volatile tLoopFlags loop;
extern volatile int16_t vbatt;
extern volatile tGains Gains;
extern volatile tCmdData CmdData;
extern volatile tSensorCal SensorCal;
extern volatile tAHRSPacket DebugPacket;
extern volatile uint16_t msSinceSpektrumByte;

/********************************* 
	main entry point
*********************************/
int main ( void )
{
    // Init the basic hardware
    InitCnsts();
    InitHardware();

    // Start the main 1Khz timer and PWM timer
    InitTimer1();
    InitTimer2();
    InitPWM();

    // Initialize A2D, 
    InitA2D();

    UART1_Init(XBEE_SPEED);         // for communication and control signals
    UART2_Init(LOGGING_RC_SPEED);   // for spektrum RC satellite receiver

    // Wait for a bit before doing rate gyro bias calibration
    // TODO: test this length of wait
    uint16_t i=0;for(i=0;i<60000;i++){Nop();}

    // turn on the leds until the bias calibration is complete
    led_on(LED_RED);
    led_on(LED_GREEN);

    // Initialize the AHRS
    AHRS_init();

    // Initialize the Controller variables
    Controller_Init();

    // MAIN CONTROL LOOP: Loop forever
    while (1)
    {
        // Gyro propagation
        if(loop.GyroProp){
            loop.GyroProp = 0;
            // Call gyro propagation
            AHRS_GyroProp();
        }

        // Attitude control
        if(loop.AttCtl){
            loop.AttCtl = 0;
            // Call attitude control
            Controller_Update();
        }

        // Accelerometer correction
        if( loop.ReadAccMag ){
            loop.ReadAccMag = 0;
            //AHRS_AccMagCorrect( );
            AHRS_ZeroCorrect();
        }

        // Send data over modem - runs at ~20Hz
        if(loop.SendSerial){
            loop.SendSerial = 0;

            // Send debug packet
            //UART1_SendAHRSpacket();
            UART1_SendAHRSEXTpacket(); //Send extended packet
        }

        // Process Spektrum RC data
        if(loop.ProcessSpektrum){
            loop.ProcessSpektrum = 0;
            UART2_ProcessSpektrumData();
        }

        // Read data from UART RX buffers - 500 Hz
        if(loop.ReadSerial){
            loop.ReadSerial = 0;

            // Read serial data
            //UART2_FlushRX_Spektrum();

        }


        // Toggle Red LED at 1Hz
        if(loop.ToggleLED){
            loop.ToggleLED = 0;
            
            // Toggle LED
            led_toggle(LED_RED);
        }

    } // End while(1)
	
}


/*---------------------------------------------------------------------
  Function Name: _T1Interrupt
  Description:   Timer1 Interrupt Handler, should execute at 1KHz
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _T1Interrupt( void )
{

    IFS0bits.T1IF = 0;  /* reset timer interrupt flag	*/

    timerCount++;

    // Increment Spektrum serial timeout counter, protecting against rollover
    if(msSinceSpektrumByte < 100)
        msSinceSpektrumByte++;
    

    // At 1000Hz, signal the gyro attitude propagation
    loop.GyroProp = 1;

    // TODO: sample accelerometers faster
    // TODO: add accelerometer bias calculation
    // TODO: write accelerometer bias to bias registers.
    // At 50Hz, signal for an acc/mag read
    if ( timerCount % 20 == 1 ){
        loop.ReadAccMag = 1;
    }

    // At 20Hz, send serial data
    if ( timerCount % 50 == 0 ){
        loop.SendSerial = 1;
    }

    // At 1Hz, LED heartbeat
    if ( timerCount % 1000 == 0 ){
        loop.ToggleLED = 1;
    }

    // At 200 Hz, read the serial buffer
    if ( timerCount % 5 == 0 ){
        loop.ReadSerial = 1;
    }

    // At 100 Hz, log data to SD card (if available)
    if ( timerCount % 10 == 0 ){
            loop.LogData = 1;
    }

}	