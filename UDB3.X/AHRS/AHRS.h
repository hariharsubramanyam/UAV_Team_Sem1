
#ifndef _AHRS_H
#define _AHRS_H

#include "../defines.h"

// Prototypes
void AHRS_init(void);
void AHRS_GyroProp(void);
void AHRS_AccMagCorrect(void);
void Controller_Update(void);
void Controller_Init(void);

// Structs
typedef struct __attribute__ ((packed)) sSensorData{

    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;

    int16_t accX;
    int16_t accY;
    int16_t accZ;

}tSensorData;

typedef struct __attribute__ ((packed)) strQuaternion{
    _Q16 o;
    _Q16 x;
    _Q16 y;
    _Q16 z;
}tQuaternion;

tQuaternion qprod(tQuaternion a, tQuaternion b);
tQuaternion qprodconj(tQuaternion a, tQuaternion b);

typedef struct __attribute__ ((packed)) sAHRSdata{
    _Q16 p;
    _Q16 q;
    _Q16 r;

    _Q16 ax;
    _Q16 ay;
    _Q16 az;

    tQuaternion q_est;
    tQuaternion q_meas;
}tAHRSdata;

typedef struct __attribute__ ((packed)) sSensorCal{

    _Q16 pBias;
    _Q16 qBias;
    _Q16 rBias;
    _Q16 gyroScale;
    _Q16 gyroPropDT;
    int16_t gyroRawBias;
    int16_t accelRawBias;

    _Q16 accelScale;
    _Q16 acc_window_min;
    _Q16 acc_window_max;
    _Q16 axBias;
    _Q16 ayBias;
    _Q16 azBias;

    _Q16 K_AttFilter;

    int16_t biasCount;
    int16_t biasTotal;
    int16_t blankReads;

}tSensorCal;

typedef struct __attribute__ ((packed)) sCmdData{
    _Q16 p;
    _Q16 q;
    _Q16 r;

    tQuaternion q_cmd;

    uint8_t throttle;
    uint8_t AttCmd;

}tCmdData;

//----------------------------------------------------------------------		

typedef struct __attribute__ ((packed)) sGains
{
    _Q16 Kp_roll;
    _Q16 Ki_roll;
    _Q16 Kd_roll;

    _Q16 Kp_pitch;
    _Q16 Ki_pitch;
    _Q16 Kd_pitch;

    _Q16 Kp_yaw;
    _Q16 Ki_yaw;
    _Q16 Kd_yaw;

    _Q16 IntRoll;
    _Q16 IntPitch;
    _Q16 IntYaw;
    _Q16 dt;

    _Q16 maxang;

}tGains;

typedef struct __attribute__ ((packed)) sRCdata{

    // motors
    uint16_t ch0;
    _Q16 ch1;
    _Q16 ch2;
    _Q16 ch3;
    uint16_t ch4;
    uint16_t ch5;
    uint16_t ch6;

}tRCdata;

#endif