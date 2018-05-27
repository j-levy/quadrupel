#include "in4073.h"
#include "switch_mode.h"

/*
Safe-mode functions for the state machine.
Jonathan Lévy
*/


char mode_0_safe_CANLEAVE(uint8_t target)
{
    char lock = 0;

    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    lock = lock || buttons;
    lock = lock || ((target != MODE_2_MANUAL) && (target != MODE_3_CALIB) && ((target != MODE_4_YAWCTRL) || !is_calibration_done));
    
    return (lock == 0);
}

char mode_0_safe_CANENTER(uint8_t source)
{
    char lock = 0;

    lock += (motor[0] != 0);
    lock += (motor[1] != 0);
    lock += (motor[2] != 0);
    lock += (motor[3] != 0);

    return (lock == 0);
}

void mode_0_safe_INIT()
{
    for (int i = 0; i < 4; i++)
        motor[i] = 0;
}

void mode_0_safe_QUIT()
{
    
}

void mode_0_safe_RUN()
{
    
}