#include "AHRS.h"

/* globals */
extern volatile tLoopFlags loop;
extern volatile tSensorCal SensorCal;
extern volatile tSensorData SensorData;
extern volatile tAHRSdata AHRSdata;
extern _Q16 num0p998, num0p0001, num1p0, num0p5, num1p1;

// Initialize
void AHRS_init(void){

    // Setup calibration struct
    SensorCal.biasCount = 0;
    SensorCal.biasTotal = 2000;
    SensorCal.blankReads = 200;
    SensorCal.pBias = _Q16ftoi(0.0);
    SensorCal.qBias = _Q16ftoi(0.0);
    SensorCal.rBias = _Q16ftoi(0.0);

    // Initialize attitude
    AHRSdata.q_est.o = _Q16ftoi(1.0);
    AHRSdata.q_est.x = _Q16ftoi(0.0);
    AHRSdata.q_est.y = _Q16ftoi(0.0);
    AHRSdata.q_est.z = _Q16ftoi(0.0);
    AHRSdata.q_meas.o = _Q16ftoi(1.0);
    AHRSdata.q_meas.x = _Q16ftoi(0.0);
    AHRSdata.q_meas.y = _Q16ftoi(0.0);
    AHRSdata.q_meas.z = _Q16ftoi(0.0);

    // Parameters
    SensorCal.gyroRawBias = 512; // Mid-range value for 10-bit A2D
    SensorCal.accelRawBias= 512; // Mid-range value for 10-bit A2D
    SensorCal.gyroScale = _Q16ftoi(PI/180.0 / 1024.0 * 3.3 / 0.0033 ); // Based on 10-bit A2D and gyro sensitivity of 3.3mV / deg/s
    SensorCal.gyroPropDT = _Q16ftoi(0.001 / 2.0);

    SensorCal.accelScale = _Q16ftoi( 3.3 / 1024.0 / 0.200 ); // at 6g resolution, sensitivity is 200mV/g
    SensorCal.acc_window_min = _Q16ftoi(0.5);
    SensorCal.acc_window_max = _Q16ftoi(1.5);

    SensorCal.K_AttFilter = _Q16ftoi(0.01*0.7); // Default value, may be overwritten by SensorPacket
}

void AHRS_GyroProp(void){
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Read raw gyro values from A2D registers, A2D automatically scans inputs at 5KHz
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    SensorData.gyroX = xgyro; SensorData.gyroX -= SensorCal.gyroRawBias;
    SensorData.gyroY = ygyro; SensorData.gyroY -= SensorCal.gyroRawBias;
    SensorData.gyroZ = zgyro; SensorData.gyroZ -= SensorCal.gyroRawBias;

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Gyro scaling, to rad/sec
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    int16toQ16(&AHRSdata.p, &SensorData.gyroX);
    AHRSdata.p = -mult( AHRSdata.p, SensorCal.gyroScale);
    int16toQ16(&AHRSdata.q, &SensorData.gyroY);
    AHRSdata.q = -mult( AHRSdata.q, SensorCal.gyroScale );
    int16toQ16(&AHRSdata.r, &SensorData.gyroZ);
    AHRSdata.r = mult( AHRSdata.r, SensorCal.gyroScale );

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Initial gyro bias calculation
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    if( SensorCal.biasCount < SensorCal.biasTotal){
        // Do some blank reads to clear any garbage in the initial transient
        if(--SensorCal.blankReads > 0)
            return;

        SensorCal.pBias += AHRSdata.p;
        SensorCal.qBias += AHRSdata.q;
        SensorCal.rBias += AHRSdata.r;

        if( ++SensorCal.biasCount == SensorCal.biasTotal ){
            _Q16 tmp = _Q16ftoi(1.0 / ((float)SensorCal.biasTotal  ));
            SensorCal.pBias = mult( SensorCal.pBias, tmp);
            SensorCal.qBias = mult( SensorCal.qBias, tmp);
            SensorCal.rBias = mult( SensorCal.rBias, tmp);
            led_off(LED_RED);
            led_off(LED_GREEN);
        }

        // TODO: Initialize q_est to q_meas

        return;
    }

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Gyro bias correction
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    AHRSdata.p -= SensorCal.pBias;
    AHRSdata.q -= SensorCal.qBias;
    AHRSdata.r -= SensorCal.rBias;

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Gyro propagation
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    AHRSdata.q_est.o -=  mult(  mult(AHRSdata.q_est.x,AHRSdata.p) +  mult(AHRSdata.q_est.y,AHRSdata.q) +  mult(AHRSdata.q_est.z,AHRSdata.r) , SensorCal.gyroPropDT);
    AHRSdata.q_est.x +=  mult(  mult(AHRSdata.q_est.o,AHRSdata.p) -  mult(AHRSdata.q_est.z,AHRSdata.q) +  mult(AHRSdata.q_est.y,AHRSdata.r) , SensorCal.gyroPropDT);
    AHRSdata.q_est.y +=  mult(  mult(AHRSdata.q_est.z,AHRSdata.p) +  mult(AHRSdata.q_est.o,AHRSdata.q) -  mult(AHRSdata.q_est.x,AHRSdata.r) , SensorCal.gyroPropDT);
    AHRSdata.q_est.z +=  mult(  mult(AHRSdata.q_est.x,AHRSdata.q) -  mult(AHRSdata.q_est.y,AHRSdata.p) +  mult(AHRSdata.q_est.o,AHRSdata.r) , SensorCal.gyroPropDT);

    // Run the attitude control after propogating gyros
    loop.AttCtl = 1;
}

void AHRS_AccMagCorrect(void)
{
    // Quit if the biases are still being calculated
    if( SensorCal.biasCount < SensorCal.biasTotal)
        return;

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Read raw accel values from A2D registers, A2D automatically scans inputs at 5KHz
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    SensorData.accX = xaccel; SensorData.accX -= SensorCal.accelRawBias;
    SensorData.accY = yaccel; SensorData.accY -= SensorCal.accelRawBias;
    SensorData.accZ = zaccel; SensorData.accZ -= SensorCal.accelRawBias;
    

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Accel scaling, to g
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    int16toQ16(&AHRSdata.ax, &SensorData.accX);
    AHRSdata.ax = mult( AHRSdata.ax, SensorCal.accelScale);
    int16toQ16(&AHRSdata.ay, &SensorData.accY);
    AHRSdata.ay = mult( AHRSdata.ay, SensorCal.accelScale );
    int16toQ16(&AHRSdata.az, &SensorData.accZ);
    AHRSdata.az = mult( AHRSdata.az, SensorCal.accelScale );
    _Q16 ax = AHRSdata.ax;
    _Q16 ay = AHRSdata.ay;
    _Q16 az = - AHRSdata.az;

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Maneuver detector, do not use accels during fast movement
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // TODO: Check angular rates

    // Roll and pitch calculation, assumes accelerometer units are 10000*g
    // Normalize the acceleration vector to length 1

    _Q16 root =  _Q16sqrt( mult(ax,ax) + mult(ay,ay) + mult(az,az));

    // TODO: Make sure we're around 1g
    if( root < SensorCal.acc_window_min || root > SensorCal.acc_window_max){
        return;
    }

    // Normalize
    ax = _IQ16div(ax, root);
    ay = _IQ16div(ay, root);
    az = _IQ16div(az, root);

    // Too close to singularity (due to numerical precision limits)
    if( ax > num0p998 || -ax > num0p998 )
        return;

    root = _Q16sqrt( mult(ay,ay) + mult(az,az));
    if(root < num0p0001 )
        root = num0p0001;

    // Calculate sin/cos of roll and pitch
    _Q16 sinR = - _IQ16div(ay,root);
    _Q16 cosR = - _IQ16div(az,root);

    _Q16 sinP = ax;
    _Q16 cosP = -( mult(ay,sinR) + mult(az,cosR) );

    // Calculate half-angles
    _Q16 cosR2 = _Q16sqrt( mult( num1p0 + cosR , num0p5  ));
    if(cosR2 < num0p0001 )
        cosR2 = num0p0001;

    _Q16 sinR2 = mult(_IQ16div( sinR , cosR2) ,  num0p5 ); // WARNING: This step is numerically ill-behaved!

    _Q16 cosP2 = _Q16sqrt( mult( num1p0 + cosP , num0p5  ));
    if(cosP2 < num0p0001 )
        cosP2 = num0p0001;

    _Q16 sinP2 = mult(_IQ16div( sinP , cosP2) ,  num0p5 ); // WARNING: This step is numerically ill-behaved!

    // Too close to singularity (due to numerical precision limits)
    if( mult(cosR2,cosR2) + mult(sinR2,sinR2) > num1p1 || mult(cosP2,cosP2) + mult(sinP2,sinP2) > num1p1 )
        return;

    // Yaw calculation
    // Normalize the magnetometer vector to length 1
/*	magx = (float)AHRSdata.magY;
    magy = -(float)AHRSdata.magX;
    magz = (float)AHRSdata.magZ;
    // Todo: magx*magx can be done in fixed pt
    root = sqrt( magx*magx + magy*magy + magz*magz );
    magx /= root;
    magy /= root;
    magz /= root;
    yaw = atan2(-cosR*magy - sinR*magz  ,  cosP*magx+sinP*sinR*magy-sinP*cosR*magz);
    yaw += PI;
    if(yaw > PI){
            yaw -= 2*PI;
    }
    sinY2 = sin(yaw/2.0);
    cosY2 = cos(yaw/2.0);
    */
    _Q16 cosY2 = _Q16ftoi(1.0);
    _Q16 sinY2 = 0;

    // Finally get quaternion
    tQuaternion qroll,qpitch,qyaw;
    qyaw.o   = cosY2; qyaw.x = 0;      qyaw.y = 0;       qyaw.z = sinY2;
    qpitch.o = cosP2; qpitch.x = 0;    qpitch.y = sinP2; qpitch.z = 0;
    qroll.o  = cosR2; qroll.x = sinR2; qroll.y = 0;      qroll.z = 0;

    AHRSdata.q_meas = qprod(qyaw,qpitch);
    AHRSdata.q_meas = qprod(AHRSdata.q_meas, qroll);

    // Check if flipped from last measurement
    if( mult(AHRSdata.q_meas.x,AHRSdata.q_est.x) + mult(AHRSdata.q_meas.y,AHRSdata.q_est.y) + mult(AHRSdata.q_meas.z,AHRSdata.q_est.z) + mult(AHRSdata.q_meas.o,AHRSdata.q_est.o) < 0 )
    {
        AHRSdata.q_meas.o = - AHRSdata.q_meas.o;
        AHRSdata.q_meas.x = - AHRSdata.q_meas.x;
        AHRSdata.q_meas.y = - AHRSdata.q_meas.y;
        AHRSdata.q_meas.z = - AHRSdata.q_meas.z;
    }

    // Gyro bias estimation

    // Make the correction
    AHRSdata.q_est.o -= mult(AHRSdata.q_est.o-AHRSdata.q_meas.o, SensorCal.K_AttFilter);
    AHRSdata.q_est.x -= mult(AHRSdata.q_est.x-AHRSdata.q_meas.x, SensorCal.K_AttFilter);
    AHRSdata.q_est.y -= mult(AHRSdata.q_est.y-AHRSdata.q_meas.y, SensorCal.K_AttFilter);
    AHRSdata.q_est.z -= mult(AHRSdata.q_est.z-AHRSdata.q_meas.z, SensorCal.K_AttFilter);

}
