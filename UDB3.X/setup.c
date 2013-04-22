#include "defines.h"
#include "Comm/Comm.h"
#include "AHRS/AHRS.h"

/****************************************
Setup hardware specs (timers, PWM, etc.)
****************************************/

/* globals */
extern volatile tLoopFlags loop;
extern volatile tAHRSdata AHRSdata;
extern volatile tSensorCal SensorCal;
extern volatile tSensorData SensorData;
extern _Q16 num24p5, num6250, num1p0;

/*---------------------------------------------------------------------
	Timer 1 should roll over at 1KHz
-----------------------------------------------------------------------*/
void InitTimer1( void )
{
    T1CON = 0;		/* ensure Timer 1 is in reset state */
    IFS0bits.T1IF = 0;	/* reset Timer 1 interrupt flag */
    IPC0bits.T1IP = 4;	/* set Timer1 interrupt priority level to 4 */
    IEC0bits.T1IE = 1;	/* enable Timer 1 interrupt */
    T1CONbits.TCKPS = 0b01;	// Prescaler = 8
    PR1 = 3685;		// Fosc/4 = 117.92MHz/4 = 29.48MHz ticks / 8 PS = 3685 KHz / 3685 Period = 1KHz rollover
    T1CONbits.TCS = 0;	/* select internal timer clock, Fosc/4 */
    T1CONbits.TON = 1;	/* enable Timer 1 and start the count */
}

/*---------------------------------------------------------------------
	Timer 2 should roll over at 490Hz - for motor PWM Generation
        1ms = 3685 timer ticks
-----------------------------------------------------------------------*/
void InitTimer2( void )
{
    T2CON = 0;		/* ensure Timer 2 is in reset state */
    T2CONbits.TCKPS = 0b01;	// Prescaler = 8
    PR2 = 7520;		// Fosc/4 = 117.92MHz/4 = 29.48MHz ticks / 8 PS = 3685 KHz / 7520 Period = 490.02 Hz rollover
    T2CONbits.TCS = 0;	/* select internal timer clock, Fosc/4 */
    T2CONbits.TON = 1;	/* enable Timer 2 and start the count */
}

/*---------------------------------------------------------------------
	Init system clock, pin directions, etc.
-----------------------------------------------------------------------*/
void InitHardware( void )
{
    /* 	Initialize pin directions */
    TRISFbits.TRISF0 = 0; // Red LED as output
    TRISFbits.TRISF1 = 0; // Green LED as output
    TRISEbits.TRISE4 = 0; // Debug pin as output
}

/*---------------------------------------------------------------------
  Function Name: InitPWM
  Description:   Start the 4 14-bit PWM generators
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void InitPWM(void)
{
    /* PWM Channel 1 */
    SETPWM(PWM1,0);	/* set initial PWM duty cycle */
    OC1CON = 0;		/* ensure output compare resister 1 is in a reset state */
    OC1CONbits.OCTSEL = 0;	/* choose timer 2 as the PWM timer */
    OC1CONbits.OCM = 0b110; /* PWM mode on OC1, fault pin disabled */

    /* PWM Channel 2 */
    SETPWM(PWM2,0);	/* set initial PWM duty cycle */
    OC2CON = 0;		/* ensure output compare resister 2 is in a reset state */
    OC2CONbits.OCTSEL = 0;	/* choose timer 2 as the PWM timer */
    OC2CONbits.OCM = 0b110; /* PWM mode on OC2, fault pin disabled */

    /* PWM Channel 3 */
    SETPWM(PWM3,0);	/* set initial PWM duty cycle */
    OC3CON = 0;		/* ensure output compare resister 3 is in a reset state */
    OC3CONbits.OCTSEL = 0;	/* choose timer 2 as the PWM timer */
    OC3CONbits.OCM = 0b110; /* PWM mode on OC3, fault pin disabled */

    /* PWM Channel 4 */
    SETPWM(PWM4,0);	/* set initial PWM duty cycle */
    OC4CON = 0;		/* ensure output compare resister 4 is in a reset state */
    OC4CONbits.OCTSEL = 0;	/* choose timer 2 as the PWM timer */
    OC4CONbits.OCM = 0b110; /* PWM mode on OC4, fault pin disabled */

}

/*---------------------------------------------------------------------
  Function Name: InitA2D
  Description:   Start the 12-bit A2D for voltage reading
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void InitA2D(void)
{
    ADCON1bits.ADSIDL = 1;     //  stop in idle mode
    ADCON1bits.FORM   = 0b00;  // integer
    ADCON1bits.SSRC   = 0b111; // auto convert after sample
    ADCON1bits.SIMSAM = 0;     // sample channels in sequence
    ADCON1bits.ASAM   = 1;     // sample again immediately after last conversion
    ADCON1bits.SAMP   = 0;     // don't sample just yet...
    ADCON1bits.DONE   = 0;     // conversion isn't done yet...

    ADCON2bits.VCFG   = 0b001;      // external vref+pin
    ADCON2bits.CSCNA  = 1;          // scan inputs
    ADCON2bits.CHPS   = 0b00;       // convert CH0
    ADCON2bits.BUFS   = 0;          // buffer fill status
    //ADCON2bits.SMPI   = 0b0110;     // interrupt every 6th (sample,read)
    ADCON2bits.SMPI   = 0b1000;     // interrupt every 8th (sample,read)
    ADCON2bits.BUFM   = 0;          // 1 16bit buffer
    ADCON2bits.ALTS   = 0;          // no alternate input mode

    ADCON3bits.SAMC   = 0b11111;    // 31Tad sample time (slowest)
    ADCON3bits.ADRC   = 0;          // system clock
    ADCON3bits.ADCS   = 0b100110;   // conversion clock - to get ~5Khz on A/D interrupt

    ADCHS  = 0b0000000000000001 ;   // channel 0 positive input is AN1
    ADPCFG = 0b1111111000000000 ;   // analog inputs on 8 7 6 5 4 3 2 1 0
    ADCSSL = 0b0000000111111111 ;   // scan inputs 8 7 6 5 4 3 2 1 0

    IFS0bits.ADIF = 0 ; 	        // clear the AD interrupt
    IPC2bits.ADIP = 4 ;             // priority 5 (1=low, 7=high)
    IEC0bits.ADIE = 0 ;             // do not enable the interrupt

    ADCON1bits.ADON = 1 ;	        // turn ON the A2D
}