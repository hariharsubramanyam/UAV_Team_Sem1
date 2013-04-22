#include "AHRS.h"
#include "../defines.h"
#include "../Comm/Comm.h"

/****************************************
Run the attitude controller at 1KHz
****************************************/

extern volatile tAHRSdata AHRSdata;
extern volatile tCmdData CmdData;
extern volatile tGains Gains;
extern _Q16 num14p45, num3685;

/*---------------------------------------------------------------------
  Function Name: Controller_Update
  Description:   Run the attitude controller, should execute at 1KHz
					Set motor values
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void Controller_Update(void)
{
    // Quaternion attitude error
    tQuaternion qerror = qprodconj(AHRSdata.q_est, CmdData.q_cmd);

    //DEBUG
    //CmdData.AttCmd = 1;
    //CmdData.throttle = 50;

    // New Quaternion Controller from Frazzoli
    _Q16 pErr = AHRSdata.p - CmdData.p;
    _Q16 qErr = AHRSdata.q - CmdData.q;
    _Q16 rErr = AHRSdata.r - CmdData.r;
    _Q16 rollCmd = mult(Gains.Kp_roll,qerror.x);
    _Q16 pitchCmd = mult(Gains.Kp_pitch,qerror.y);
    _Q16 yawCmd = mult(Gains.Kp_yaw,qerror.z);
    if (qerror.o < 0){
        rollCmd = -rollCmd;
        pitchCmd = -pitchCmd;
        yawCmd = -yawCmd;
    }

    // Increment Integrators if motor commands are being sent
    if (1 == CmdData.AttCmd || 2 == CmdData.AttCmd) {
        Gains.IntRoll += mult(rollCmd,Gains.dt);
        Gains.IntPitch += mult(pitchCmd,Gains.dt);
        Gains.IntYaw += mult(yawCmd,Gains.dt);
    }

    rollCmd += mult(Gains.Ki_roll,Gains.IntRoll) - mult(Gains.Kd_roll,pErr);
    pitchCmd += mult(Gains.Ki_pitch,Gains.IntPitch) - mult(Gains.Kd_pitch,qErr);
    yawCmd += mult(Gains.Ki_yaw,Gains.IntYaw) - mult(Gains.Kd_yaw,rErr);
    // End New Quaternion controller

    // Form motor commands
    _Q16 m1 = -pitchCmd - yawCmd;
    _Q16 m2 = -rollCmd  + yawCmd;
    _Q16 m3 =  pitchCmd - yawCmd;
    _Q16 m4 =  rollCmd  + yawCmd;

    // DEBUG
    CmdData.throttle = 255;
    CmdData.AttCmd = 1;
    // END DEBUG

    // Get throttle value
    int16_t tmp = (int16_t) CmdData.throttle;
    _Q16 throttle = 0;
    int16toQ16(&throttle,&tmp);     // Throttle between 0.0 and 255.0 here

    m1 = 0; m2 = 0; m3 = 0; m4 = 0;

    if (1 == CmdData.AttCmd || 2 == CmdData.AttCmd) {

        /**** PWM motors - take value between 0 and 3685 (converted to 1 <--> 2 ms PWM signal at 490Hz)*/
        int16_t pwmData = 0;
        throttle = mult(throttle,num14p45);  // multiply throttle by 14.45 to get 0 to 3685  ( assuming throttle is between 0 and 255)
        _Q16 motorCmd = m1 + throttle;
        _Q16sat(&motorCmd, num3685, 0);     // TODO: find lower saturation bound here so motors don't turn off
        Q16toint16(&pwmData,&motorCmd);
        SETPWM(PWM1,pwmData);

        motorCmd = m2 + throttle;
        _Q16sat(&motorCmd, num3685, 0);     // TODO: find lower saturation bound here so motors don't turn off
        Q16toint16(&pwmData,&motorCmd);
        SETPWM(PWM2,pwmData);

        motorCmd = m3 + throttle;
        _Q16sat(&motorCmd, num3685, 0);     // TODO: find lower saturation bound here so motors don't turn off
        Q16toint16(&pwmData,&motorCmd);
        SETPWM(PWM3,pwmData);

        motorCmd = m4 + throttle;
        _Q16sat(&motorCmd, num3685, 0);     // TODO: find lower saturation bound here so motors don't turn off
        Q16toint16(&pwmData,&motorCmd);
        SETPWM(PWM4,pwmData);

 

    } else if (0 == CmdData.AttCmd) {

      	// set motor values to off
        SETPWM(PWM1,0);
        SETPWM(PWM2,0);
        SETPWM(PWM3,0);
        SETPWM(PWM4,0);

    }

}

/*---------------------------------------------------------------------
  Function Name: Controller_Init
  Description:   Initialize all the controller variables
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void Controller_Init(void)
{
    // ESCs
    SETPWM(PWM1,0);
    SETPWM(PWM2,0);
    SETPWM(PWM3,0);
    SETPWM(PWM4,0);

    CmdData.q_cmd.o = _Q16ftoi(1.0);
    CmdData.q_cmd.x = _Q16ftoi(0.0);
    CmdData.q_cmd.y = _Q16ftoi(0.0);
    CmdData.q_cmd.z = _Q16ftoi(0.0);
    CmdData.p = _Q16ftoi(0.0);
    CmdData.q = _Q16ftoi(0.0);
    CmdData.r = _Q16ftoi(0.0);
    CmdData.throttle = 0;
    CmdData.collective = 0;
    CmdData.AttCmd = 0;

    // mQxx default values
    Gains.Kp_roll = _Q16ftoi(354.0);
    Gains.Ki_roll = _Q16ftoi(0.0);
    Gains.Kd_roll = _Q16ftoi(118.0);

    Gains.Kp_pitch = _Q16ftoi(354.0);
    Gains.Ki_pitch = _Q16ftoi(0.0);
    Gains.Kd_pitch = _Q16ftoi(118.0);

    Gains.Kp_yaw = _Q16ftoi(0.0);
    Gains.Ki_yaw = _Q16ftoi(0.0);
    Gains.Kd_yaw = _Q16ftoi(184.0);

    Gains.maxang = _Q16ftoi(0.8);
    Gains.lowBatt = 7200; // 7.2 volts
    Gains.stream_data = 0;

    Gains.IntRoll = _Q16ftoi(0.0);
    Gains.IntPitch = _Q16ftoi(0.0);
    Gains.IntYaw = _Q16ftoi(0.0);
    Gains.dt = _Q16ftoi(1.0/1000.0);
}
//======================================================================