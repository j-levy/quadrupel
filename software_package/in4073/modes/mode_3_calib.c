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
    is_calibration_done = true;

    // initialize to last read values.
    sax_offset = sax;
    say_offset = say;
    saz_offset = saz;
    sp_offset = sp;
    sq_offset = sq;
    sr_offset = sr;
    phi_offset = phi;
    theta_offset = theta;
    psi_offset = psi;

}

void mode_3_calib_QUIT()
{
    // nothing in particular.
}

void mode_3_calib_RUN()
{    
    // get values, store them as offsets. 
    // The board might move, so we do this repetively so that only the final value is registered.
    // the sensor reading is already done in the main, actually.


    // make a mean over time, but without taking hundreds of bytes of memory...
    sax_offset = (sax/2) + (sax_offset/2);
    say_offset = (say/2) + (sax_offset/2);
    saz_offset = (saz/2)+ (sax_offset/2);
    sp_offset = (sp/2) + (sp_offset/2);
    sq_offset = (sq/2) + (sq_offset/2);
    sr_offset = (sr/2) + (sr_offset/2);
    phi_offset = (phi/2) + (phi_offset/2);
    theta_offset = (theta/2) + (theta_offset/2);
    psi_offset = (psi/2) + (psi_offset/2);
    printf("DMP Calib |sax|%i|say|%i|saz|%i|sp|%d|sq|%d|sr|%d|\n",sax,say,saz,sp,sq,sr);

}