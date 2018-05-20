#include "in4073.h"
#include "switch_mode.h"

/*
Calibration functions for the state machine.
Jonathan Lévy
*/


char mode_3_calib_CANLEAVE(uint8_t target)
{
    char lock = 0;

    // is the joystick still ?
    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    // is there no button pressed ?
    lock = lock || buttons;
    
    return (lock == 0);

}

char mode_3_calib_CANENTER(uint8_t source)
{
    char lock = 0;

    // are all the motors off ?
    lock += (motor[0] != 0);
    lock += (motor[1] != 0);
    lock += (motor[2] != 0);
    lock += (motor[3] != 0);

    return (lock == 0);
}

void mode_3_calib_INIT()
{
    // nothing in particular.
}

void mode_3_calib_QUIT()
{
    // nothing in particular.
}

void mode_3_calib_RUN()
{    
    // get values, store them as offsets. 
    // The board might move, so we do this repetively so that only the final value is registered.

    if (is_DMP_on)
        get_dmp_data();
    else
        get_raw_sensor_data();
    
    sax_offset = sax;
    say_offset = say;
    saz_offset = saz;
    sp_offset = sp;
    sq_offset = sq;
    sr_offset = sr;

}