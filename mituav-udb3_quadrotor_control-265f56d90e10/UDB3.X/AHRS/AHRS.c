#include "AHRS.h"

/* globals */
extern volatile tLoopFlags loop;
extern volatile tSensorCal SensorCal;
extern volatile tSensorData SensorData;
extern volatile tAHRSdata AHRSdata;
extern _Q16 num0p998, num0p0001, num0p05, num0p1, num0p2, num0p5, num0p8, num0p9, num0p95, num1p0, num1p1;

// Initialize
void AHRS_init(void){

    // Setup calibration struct
    SensorCal.biasCountGyro = 0;
    SensorCal.biasTotalGyro = 2000;
    SensorCal.blankReadsGyro = 200;
    SensorCal.biasCountAcc = 0;
    SensorCal.biasTotalAcc = 200;
    SensorCal.blankReadsAcc = 50;
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

    AHRSdata.q_zero.o = _Q16ftoi(1.0);
    AHRSdata.q_zero.x = _Q16ftoi(0.0);
    AHRSdata.q_zero.y = _Q16ftoi(0.0);
    AHRSdata.q_zero.z = _Q16ftoi(0.0);

    // Parameters
    SensorCal.gyroRawBias = 512; // Mid-range value for 10-bit A2D
    SensorCal.accelRawBias= 500; // Mid-range value for 10-bit A2D (adjusted manually)
    SensorCal.gyroScale = _Q16ftoi(PI/180.0 / 1024.0 * 3.3 / 0.0033  * 1.5 ); // Based on 10-bit A2D and gyro sensitivity of 3.3mV / deg/s with fudge factor
    SensorCal.gyroPropDT = _Q16ftoi(0.001 / 2.0);

    SensorCal.accelScale = _Q16ftoi( 3.3 / 1024.0 / 0.200 ); // at 6g resolution, sensitivity is 200mV/g
    SensorCal.acc_window_min = _Q16ftoi(0.5);
    SensorCal.acc_window_max = _Q16ftoi(1.5);
    SensorCal.axBias = _Q16ftoi(0.0);
    SensorCal.ayBias = _Q16ftoi(0.0);
    SensorCal.azBias = _Q16ftoi(0.0);
    SensorCal.accFiltA = _Q16ftoi( 0.9 );
    SensorCal.accFiltB = _Q16ftoi( 0.1 );
    SensorCal.correctSatLow = _Q16ftoi( -0.025 );
    SensorCal.correctSatHigh = _Q16ftoi( 0.025 );


    //SensorCal.K_AttFilter = _Q16ftoi(0.025); // Default value, may be overwritten by SensorPacket
    SensorCal.K_AttFilter = _Q16ftoi(0.01); //Debug: No Acc correction
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
    AHRSdata.q = mult( AHRSdata.q, SensorCal.gyroScale );
    int16toQ16(&AHRSdata.r, &SensorData.gyroZ);
    AHRSdata.r = mult( AHRSdata.r, SensorCal.gyroScale );

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Initial gyro bias calculation
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    if( SensorCal.biasCountGyro < SensorCal.biasTotalGyro){
        // Do some blank reads to clear any garbage in the initial transient
        if(--SensorCal.blankReadsGyro > 0)
            return;

        SensorCal.pBias += AHRSdata.p;
        SensorCal.qBias += AHRSdata.q;
        SensorCal.rBias += AHRSdata.r;

        led_on(LED_RED);

        if( ++SensorCal.biasCountGyro == SensorCal.biasTotalGyro ){
            _Q16 tmp = _Q16ftoi(1.0 / ((float)SensorCal.biasTotalGyro  ));
            //SensorCal.pBias = mult( SensorCal.pBias, tmp);
            //SensorCal.qBias = mult( SensorCal.qBias, tmp);
            //SensorCal.rBias = mult( SensorCal.rBias, tmp);
            
            tmp = _Q16ftoi((float)SensorCal.biasTotalGyro );
            SensorCal.pBias = _IQ16div( SensorCal.pBias, tmp );
            SensorCal.qBias = _IQ16div( SensorCal.qBias, tmp );
            SensorCal.rBias = _IQ16div( SensorCal.rBias, tmp );

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
    AHRSdata.ax = -mult( AHRSdata.ax, SensorCal.accelScale);
    int16toQ16(&AHRSdata.ay, &SensorData.accY);
    AHRSdata.ay = mult( AHRSdata.ay, SensorCal.accelScale );
    int16toQ16(&AHRSdata.az, &SensorData.accZ);
    AHRSdata.az = -mult( AHRSdata.az, SensorCal.accelScale );


    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Initial acc bias calculation
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    if( SensorCal.biasCountAcc < SensorCal.biasTotalAcc){
        // Do some blank reads to clear any garbage in the initial transient
        if(--SensorCal.blankReadsAcc > 0)
            return;

        SensorCal.axBias += AHRSdata.ax;
        SensorCal.ayBias += AHRSdata.ay;
        SensorCal.azBias += AHRSdata.az;

        led_on(LED_RED);

        if( ++SensorCal.biasCountAcc == SensorCal.biasTotalAcc ){
            _Q16 tmp = _Q16ftoi(1.0 / ((float)SensorCal.biasTotalAcc  ));
            SensorCal.axBias = mult( SensorCal.axBias, tmp);
            SensorCal.ayBias = mult( SensorCal.ayBias, tmp);
            //Z is different, and hast to be biased by 1g
            SensorCal.azBias = mult( SensorCal.azBias, tmp) + _Q16ftoi(1.0);
            led_off(LED_RED);
            led_off(LED_GREEN);
        }


        return;
    }

    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    // Acc bias correction
    // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


    AHRSdata.ax = AHRSdata.ax - SensorCal.axBias;
    AHRSdata.axavg = mult(AHRSdata.ax, SensorCal.accFiltA) + mult(AHRSdata.axavg, SensorCal.accFiltB);
    AHRSdata.ay = AHRSdata.ay - SensorCal.ayBias;
    AHRSdata.ayavg = mult(AHRSdata.ay, SensorCal.accFiltA) + mult(AHRSdata.ayavg, SensorCal.accFiltB);
    AHRSdata.az = AHRSdata.az - SensorCal.azBias;
    AHRSdata.azavg = mult(AHRSdata.az, SensorCal.accFiltA) + mult(AHRSdata.azavg, SensorCal.accFiltB);

    _Q16 ax = AHRSdata.axavg;
    _Q16 ay = AHRSdata.ayavg;
    _Q16 az = AHRSdata.azavg;

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


volatile tQuaternion qerr;
void AHRS_ZeroCorrect( void )
{
     // Check if flipped from last measurement
    if( mult(AHRSdata.q_zero.x,AHRSdata.q_est.x) + mult(AHRSdata.q_zero.y,AHRSdata.q_est.y) + mult(AHRSdata.q_zero.z,AHRSdata.q_est.z) + mult(AHRSdata.q_zero.o,AHRSdata.q_est.o) < 0 )
    {
        AHRSdata.q_zero.o = - AHRSdata.q_zero.o;
        AHRSdata.q_zero.x = - AHRSdata.q_zero.x;
        AHRSdata.q_zero.y = - AHRSdata.q_zero.y;
        AHRSdata.q_zero.z = - AHRSdata.q_zero.z;
    }


    qerr.o = AHRSdata.q_est.o-AHRSdata.q_zero.o;
    qerr.x = AHRSdata.q_est.x-AHRSdata.q_zero.x;
    qerr.y = AHRSdata.q_est.y-AHRSdata.q_zero.y;
    qerr.z = AHRSdata.q_est.z-AHRSdata.q_zero.z;



    //_Q16sat(&qerr.o, SensorCal.correctSatHigh, SensorCal.correctSatLow);
    //_Q16sat(&qerr.x, SensorCal.correctSatHigh, SensorCal.correctSatLow);
    //_Q16sat(&qerr.y, SensorCal.correctSatHigh, SensorCal.correctSatLow);
    //_Q16sat(&qerr.z, SensorCal.correctSatHigh, SensorCal.correctSatLow);

    // Make the correction
    AHRSdata.q_est.o -= mult(qerr.o , SensorCal.K_AttFilter);
    AHRSdata.q_est.x -= mult(qerr.x, SensorCal.K_AttFilter);
    AHRSdata.q_est.y -= mult(qerr.y, SensorCal.K_AttFilter);
    AHRSdata.q_est.z -= mult(qerr.z, SensorCal.K_AttFilter);
}
