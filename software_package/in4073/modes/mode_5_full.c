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
    for (int i = 0; i < 4; i++) 
        flight_coeffs[i] = 1;
    
    // coefficients determined empirically. They seem more or less ok.
    flight_coeffs[ROLL] = 9;
    flight_coeffs[PITCH] = flight_coeffs[ROLL];
    flight_coeffs[LIFT] = 3*flight_coeffs[ROLL];
    flight_coeffs[YAW] = 2*flight_coeffs[ROLL];

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
    int32_t oo[4];
    int32_t js[4];
    int32_t a[4];
    
    js[ROLL]= axis[ROLL] ;
    js[PITCH] = axis[PITCH] ;
    js[YAW] = axis[YAW] ;
    js[LIFT] = (-(axis[LIFT] - 32767) >> 1);

    /* ##################################################
    ################ YAW CONTROL HERE ################### 
    ################################################### */
    //code freely inspired from the simulator.
    
    
    js[YAW] = p_yaw * (js[YAW]/p_yaw - __SR);

    telemetry_packet[PHI] = MSBYTE(__SR);
    telemetry_packet[PHI+1] = LSBYTE(__SR);

    /* ##################################################
    ################ END OF YAW CONTROL ################# 
    ################################################### */
  

   

    /*###################################################
    ###############  ROLL CONTROL PART  #################
    ###################################################*/

    // telemetry_packet[PHI] = MSBYTE(__PHI);
    // telemetry_packet[PHI+1] = LSBYTE(__PHI);
    telemetry_packet[SETPOINT_ROLL] = MSBYTE(js[ROLL]);
    telemetry_packet[SETPOINT_ROLL+1] = LSBYTE(js[ROLL]);

    js[ROLL] = p_p2 * (p_p1 * (js[ROLL]/(p_p1*p_p2) - __PHI) - __SP);
    /*###################################################
    ##############  END OF ROLL CONTROL  ################
    ##################################################*/



    /*###################################################
    ###############  PITCH CONTROL PART  ################
    ###################################################*/

    /*
    //This part introduces cascade p controller
    //Equations as per kalman_control.pdf

    int32_t setpoint_pitch_s = js_pitch;

    int32_t setpoint_q_s;
    setpoint_q_s = p_p1 * (setpoint_pitch_s - __THETA);

    js_pitch = p_p2 * (setpoint_q_s - __SQ);
    */

    js[PITCH] = p_p2 * (p_p1 * (js[PITCH]/(p_p1*p_p2) - __THETA) - __SQ);
    /*###################################################
    ##############  END OF PITCH CONTROL  ###############
    ##################################################*/

    
    for (int i = 0; i < 4; i++)
    {
        a[i] = offset[i] + js[i];
    }

    oo[0] = a[LIFT]*flight_coeffs[LIFT] + 2 * a[PITCH]*flight_coeffs[PITCH] - a[YAW]*flight_coeffs[YAW] ;
	oo[1] = a[LIFT]*flight_coeffs[LIFT] - 2 * a[ROLL]*flight_coeffs[ROLL] + a[YAW]*flight_coeffs[YAW];
	oo[2] = a[LIFT]*flight_coeffs[LIFT] - 2 * a[PITCH]*flight_coeffs[PITCH] - a[YAW]*flight_coeffs[YAW] ;
	oo[3] = a[LIFT]*flight_coeffs[LIFT] + 2 * a[ROLL]*flight_coeffs[ROLL] + a[YAW]*flight_coeffs[YAW] ;

    for (int i = 0; i < 4 ; i++)
    {
        oo[i] = oo[i] / ( flight_coeffs[LIFT] + flight_coeffs[PITCH] + flight_coeffs[ROLL] + flight_coeffs[YAW]); 
        // because oo_max = (sum_coeffs) * coeffs)*a_max
        // because coeff[ROLL] == coeff[PITCH] (I do this because of symmetry, makes sense)
        oo[i] = MAX(oo[i], 0);
        oo[i] = (oo[i] < MIN_SPEED*32 ? MIN(a[LIFT], MIN_SPEED*32) : oo[i]);
        oo[i] = MIN(oo[i], MAX_SPEED*32);
        ae[i] = oo[i] / (32); // scale 0->32767 to 0->1023. But is capped anyway. So it's a good scale.
    }

    update_motors();
   
}