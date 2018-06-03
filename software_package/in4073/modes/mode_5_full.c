/*
MODE 5: yaw control function.
Author:Yuhao Xuan
*/
#include "in4073.h"
#include "switch_mode.h"
#include "mode_2_manual.h"
#include "mode_4_yaw.h"
#include <math.h>
// TODO: rename mode_2_manual into something more generic, as it contains data for multiple modes actually.
// TODO: rename mode_4_yaw into something more generic (same)


// DEPRECATED
#define JS_BITSCALE 1
#define BITSCALE 6



char mode_5_full_CANLEAVE(uint8_t target)
{
    char lock = 0;
    lock = (target != MODE_1_PANIC) && (target != MODE_0_SAFE);
    return (lock == 0);
}

char mode_5_full_CANENTER(uint8_t source)
{
    char lock = 0;

    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    lock = lock || buttons;
    
    return (lock == 0);
}
void mode_5_full_INIT()
{
    // initialize the global variable for the controller.
    p_yaw = 1;
    p_p1 = 1;
    p_p2 = 1;
}

void mode_5_full_QUIT()
{
// for now nothing to be done
}

void mode_5_full_RUN(void)

{
    int32_t oo1, oo2, oo3, oo4;
    int32_t js_roll, js_pitch, js_lift, js_yaw, a_roll, a_pitch, a_yaw, a_lift;
    
    // joystick reading.
    // js_roll = axis[ROLL] >> (BITSCALE);
    // js_pitch = axis[PITCH] >> (BITSCALE);
    // js_yaw = ((axis[YAW]) * DT) >> BITSCALE;
    // js_lift = (-(axis[LIFT] - 32767) >> 1) >> BITSCALE;

    // joystick reading.
    js_roll = axis[ROLL] >> JS_BITSCALE;
    js_pitch = axis[PITCH] >> JS_BITSCALE;
    js_yaw = axis[YAW] >> JS_BITSCALE;
    js_lift = (-(axis[LIFT] - 32767) >> 1);

    /* ##################################################
    ################ YAW CONTROL HERE ################### 
    ################################################### */
    //code freely inspired from the simulator.
    
    //read sensor data
    // get_dmp_data(); // reads variable "sr"
    
    int32_t setpoint_r = js_yaw; // setpoint is angular rate
    js_yaw = p_yaw * (setpoint_r/4 - __SR);

    telemetry_packet[PHI] = MSBYTE(__SR);
    telemetry_packet[PHI+1] = LSBYTE(__SR);

    /* ##################################################
    ################ END OF YAW CONTROL ################# 
    ################################################### */
  

   

    /*###################################################
    ###############  ROLL CONTROL PART  #################
    ###################################################*/

    //This part introduces cascade p controller
    //Equations as per kalman_control.pdf
    int32_t setpoint_roll_s = js_roll;

    int32_t setpoint_p_s;
    setpoint_p_s = p_p1 * (setpoint_roll_s - __PHI);

    // telemetry_packet[PHI] = MSBYTE(__PHI);
    // telemetry_packet[PHI+1] = LSBYTE(__PHI);
    telemetry_packet[SETPOINT_ROLL] = MSBYTE(setpoint_roll_s);
    telemetry_packet[SETPOINT_ROLL+1] = LSBYTE(setpoint_roll_s);

    js_roll = p_p2 * (setpoint_p_s - __SP);
    /*###################################################
    ##############  END OF ROLL CONTROL  ################
    ##################################################*/



    /*###################################################
    ###############  PITCH CONTROL PART  ################
    ###################################################*/

    //This part introduces cascade p controller
    //Equations as per kalman_control.pdf
    int32_t setpoint_pitch_s = js_pitch;

    int32_t setpoint_q_s;
    setpoint_q_s = p_p1 * (setpoint_pitch_s - __THETA);

    js_pitch = p_p2 * (setpoint_q_s - __SQ);
    /*###################################################
    ##############  END OF PITCH CONTROL  ###############
    ##################################################*/


    // // adding offsets
    // a_roll = offset[ROLL] + js_roll;
    // a_pitch = offset[PITCH] + js_pitch;
    // a_yaw = (offset[YAW] + js_yaw) >> 2; // make the yaw rate smaller to the change, so that we can asee something.
    // a_lift = offset[LIFT] + js_lift;



    // // computing speeds, manual style.
    // oo1 = (a_lift + 2 * a_pitch - a_yaw) ;
	// oo2 = (a_lift - 2 * a_roll + a_yaw);
	// oo3 = (a_lift - 2 * a_pitch - a_yaw) ;
	// oo4 = (a_lift + 2 * a_roll + a_yaw);

    // /* Here, limit the motor speeds after computation, then update motors!*/

    // oo1 = (oo1 < 200 ? MIN(a_lift, 200) : oo1);
    // oo2 = (oo2 < 200? MIN(a_lift, 200) : oo2);
    // oo3 = (oo3 < 200 ? MIN(a_lift, 200) : oo3);
    // oo4 = (oo4 < 200? MIN(a_lift, 200) : oo4);

    // if (oo1 > MAX_SPEED) oo1 = MAX_SPEED;
	// if (oo2 > MAX_SPEED) oo2 = MAX_SPEED;
	// if (oo3 > MAX_SPEED) oo3 = MAX_SPEED;
	// if (oo4 > MAX_SPEED) oo4 = MAX_SPEED;

	// ae[0] = (oo1) >> (BITSCALE);
	// ae[1] = (oo2) >> (BITSCALE);
	// ae[2] = (oo3) >> (BITSCALE);
	// ae[3] = (oo4) >> (BITSCALE);


    // new motor speed assignment

    a_roll = offset[ROLL] + js_roll;
    a_pitch = offset[PITCH] + js_pitch;
    a_yaw = offset[YAW] + js_yaw;
    a_lift = offset[LIFT] + js_lift;

    oo1 = (a_lift + 2 * a_pitch - a_yaw) ;
	oo2 = (a_lift - 2 * a_roll + a_yaw) ;
	oo3 = (a_lift - 2 * a_pitch - a_yaw) ;
	oo4 = (a_lift + 2 * a_roll + a_yaw) ;



	oo1 = (oo1) >> (BITSCALE);
	oo2 = (oo2) >> (BITSCALE);
	oo3 = (oo3) >> (BITSCALE);
    oo4 = (oo4) >> (BITSCALE);


    oo1 = (oo1 < 200 ? MIN(a_lift>>BITSCALE, 200) : oo1);
    oo2 = (oo2 < 200? MIN(a_lift>>BITSCALE, 200) : oo2);
    oo3 = (oo3 < 200 ? MIN(a_lift>>BITSCALE, 200) : oo3);
    oo4 = (oo4 < 200? MIN(a_lift>>BITSCALE, 200) : oo4);


    oo1 = MAX(0, oo1);
    oo2 = MAX(0, oo2);
    oo3 = MAX(0, oo3);
    oo4 = MAX(0, oo4);

	/* clip ooi as rotors only provide prositive thrust
	 */
	

    if (oo1 > MAX_SPEED) oo1 = MAX_SPEED;
	if (oo2 > MAX_SPEED) oo2 = MAX_SPEED;
	if (oo3 > MAX_SPEED) oo3 = MAX_SPEED;
	if (oo4 > MAX_SPEED) oo4 = MAX_SPEED;

	/* with ai = oi it follows
	 */
	ae[0] = (oo1);
	ae[1] = (oo2);
	ae[2] = (oo3);
	ae[3] = (oo4);



    update_motors();
   
}