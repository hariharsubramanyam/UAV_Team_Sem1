#ifndef _DEFINES_H
#define _DEFINES_H

#include "p30f4011.h"			// generic file header
#include <libq.h>

// Settings for DSM satellite receiver
#define DSMX   // If using DSMX receiver

// DEFAULT CONTROLLER SETTINGS -- only important if flying in RC mode
#define MQXX          // load default control values for mini quads

#define SONAR_SPEED         9600        // serial speed for maxboxic sonar
#define LOGGING_RC_SPEED    115200      // serial speed for openlog data logger and spekrum rc receiver
#define XBEE_SPEED          57600       // serial speed for xbee wireless radio

// Clock speed 117.92MHz (7.37MHz Fast RC internal osc w/ 16x PLL)
#define FCY 117920000

// A2D pins
#define	xgyro  ADCBUF1
#define	ygyro  ADCBUF3
#define	zgyro  ADCBUF2
#define	xaccel ADCBUF6 //ADCBUF4
#define	yaccel ADCBUF7 //ADCBUF5
#define	zaccel ADCBUF8 //ADCBUF6
#define noconn ADCBUF4
#define noconn2  ADCBUF5

#define PI 3.14159265

// Type defs
#define uint8_t 	unsigned char
#define BYTE 		unsigned char
#define uint16_t 	unsigned int
#define int16_t		int
#define uint32_t	long unsigned int
#define int32_t		long int
#define int8_t		char			// 127 -> -128


// Pin defs
// LED pins
#define LED_OFF				1
#define LED_ON				0
#define led_toggle(x)		((x) = !(x))
#define led_on(x)               ((x)= LED_ON  )
#define led_off(x)               ((x)= LED_OFF  )
#define LED_RED			LATFbits.LATF0
#define LED_GREEN		LATFbits.LATF1

// Switch (SR3)
#define SWITCH3 PORTFbits.RF6

// Output pin for debugging
#define DBG_PIN			LATEbits.LATE4 // debug pin - RE4

// Servo defines, to match UDB3 Radio port numbers
#define PWM1				OC3RS
#define PWM2				OC4RS
#define PWM3				OC2RS
#define PWM4				OC1RS
#define SETPWM(channel,x)	(channel = (x + 3685))

typedef struct sLoopFlags{
    uint8_t GyroProp;
    uint8_t AccMagCorrect;
    uint8_t AttCtl;
    uint8_t SendSerial;
    uint8_t SendSensors;
    uint8_t ReadSerial;
    uint8_t StartWait;
    uint8_t ToggleLED;
    uint8_t ReadAccMag;
    uint8_t I2C1Recover;
    uint8_t I2C2Recover;
    uint8_t LogData;
    uint8_t ProcessSpektrum;
}tLoopFlags;


// Some helpful functions
void int16toQ16(_Q16 *to, int16_t *from);
_Q16 mult( _Q16 a, _Q16 b);
_Q16 _Q16sqrt(_Q16 in);
void _Q16sat(_Q16 * val, _Q16 high, _Q16 low); 
void uint8sat(uint8_t * val, uint8_t high, uint8_t low);
void int16sat(int16_t * val, int16_t high, int16_t low);
void Q16touint8(uint8_t *to, _Q16 *from);
void Q16toint16(int16_t *to, _Q16 *from);
void _Q16abs(_Q16 *val);

// C Prototype for function in div16.s
 _Q16 _IQ16div(_Q16, _Q16);

// Hardware initialization prototypes
void InitTimer1();
void InitTimer2();
void InitHardware();	
void InitPWM();
void InitA2D();
void InitCnsts();
void InitCnsts();

// Test Sensors
void SensorTest();

#endif

