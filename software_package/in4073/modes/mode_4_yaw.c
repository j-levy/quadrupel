/*
    MODE 4: yaw control.
    (Most of the useful code is in control.c)
*/
#include "in4073.h"
#include "switch_mode.h"
#include "modes_flight.h"




char mode_4_yaw_CANLEAVE(uint8_t target)
{
    char lock = 0;
    lock = (target != MODE_1_PANIC) && (target != MODE_0_SAFE);
    return (lock == 0);
}

char mode_4_yaw_CANENTER(uint8_t source)
{
    char lock = 0;

    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    lock = lock || buttons;
    
    return (lock == 0);
}
void mode_4_yaw_INIT()
{
    
    // coefficients determined empirically. They seem more or less ok.
    flight_coeffs[ROLL] = 9;
    flight_coeffs[PITCH] = flight_coeffs[ROLL];
    flight_coeffs[LIFT] = 3*flight_coeffs[ROLL];
    flight_coeffs[YAW] = 2*flight_coeffs[ROLL];

    // initialize the global variable for the controller.
    p_yaw = P_SCALE;
}

void mode_4_yaw_QUIT()
{
    // for now nothing to be done
}

void mode_4_yaw_RUN(void)

{
    // code deported - not to be run at crazy speed
}