#include "defines.h"
#include "AHRS/AHRS.h"
#include "Comm/Comm.h"

// Loop flags
volatile tLoopFlags loop;
volatile unsigned long timerCount = 0;

// Sensor data
volatile tSensorData SensorData;
volatile tSensorCal SensorCal;
volatile tAHRSdata AHRSdata;
volatile int16_t vbatt;		// battery voltage in mV
volatile tRCdata RCdata;
volatile tDebugPacket DebugPacket;

// Controller
volatile tCmdData CmdData; 
volatile tGains Gains;

// Hack -- Convert these numbers at startup to save execution time:
_Q16 num0p998, num0p0001, num0p001, num1p0, num0p5, 
        num1p1, num2p0, num4p0, num255, numPI, num2125, num875,
        num625, num1500, num2000, num1000, num500, num512,
        num3685, num14p45;