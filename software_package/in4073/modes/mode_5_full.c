/*
MODE 5: yaw control function.
Author:Yuhao Xuan, Jonathan Levy
*/
#include "in4073.h"
#include "switch_mode.h"
#include "modes_flight.h"


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
    /*
    flight_coeffs[ROLL] = 9;
    flight_coeffs[PITCH] = flight_coeffs[ROLL];
    flight_coeffs[LIFT] = 3*flight_coeffs[ROLL];
    flight_coeffs[YAW] = 2*flight_coeffs[ROLL];
*/
    flight_coeffs[ROLL] = 7;
    flight_coeffs[PITCH] = flight_coeffs[ROLL];
    flight_coeffs[LIFT] = 29;
    flight_coeffs[YAW] = 15;

    // initialize the global variable for the controller.
    p_yaw = P_SCALE;
    p_p1 = P_SCALE;
    p_p2 = P_SCALE;
}

void mode_5_full_QUIT()
{
// for now nothing to be done
}

void mode_5_full_RUN(void)

{
    // code deported - not to be run at crazy speed
}