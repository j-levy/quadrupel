#include "in4073.h"
#include "switch_mode.h"
/*
Manual mode functions for the state machine.
Jonathan Lévy
*/

char mode_2_manual_CANLEAVE(uint8_t target)
{
    char lock = 0;
    lock = (target != MODE_1_PANIC) && (target != MODE_0_SAFE);
    return (lock == 0);
}

char mode_2_manual_CANENTER(uint8_t source)
{
    char lock = 0;

    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    lock = lock || buttons;
    
    return (lock == 0);
}


void mode_2_manual_INIT()
{
    for (int i = 0; i < 4; i++) 
        flight_coeffs[i] = 1;
    
    // coefficients determined empirically. They seem more or less ok.
    flight_coeffs[ROLL] = 9;
    flight_coeffs[PITCH] = flight_coeffs[ROLL];
    flight_coeffs[LIFT] = 3*flight_coeffs[ROLL];
    flight_coeffs[YAW] = 2*flight_coeffs[ROLL];

    sp_offset = 0;
    sq_offset = 0;
    sr_offset = 0; 
    sax_offset = 0;
    say_offset = 0;
    saz_offset = 0;
    phi_offset = 0;
    psi_offset = 0; 
    theta_offset = 0;

}

void mode_2_manual_QUIT()
{
    
}

void mode_2_manual_RUN()
{
    // code deported - not to be run at crazy speed
}