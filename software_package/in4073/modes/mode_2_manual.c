#include "in4073.h"
#include "switch_mode.h"
#include "mode_2_manual.h"

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

}

void mode_2_manual_QUIT()
{
    
}

void mode_2_manual_RUN()
{
    int16_t oo1, oo2, oo3, oo4;
    int16_t js_roll, js_pitch, js_lift, a_roll, a_pitch, a_yaw, a_lift;
    static int16_t js_yaw = 0;
    
    js_roll = axis[ROLL] >> (BITSCALE+2);
    js_pitch = axis[PITCH] >> (BITSCALE+2);

    // Yaw command is not cumulative in manual mode
    js_yaw = ((axis[YAW]) * DT) >> BITSCALE;
    js_lift = (-(axis[LIFT] - 32767) >> 1) >> BITSCALE;

    a_roll = offset[ROLL] + js_roll;
    a_pitch = offset[PITCH] + js_pitch;
    a_yaw = offset[YAW] + js_yaw;
    a_lift = offset[LIFT] + js_lift;

    // a_roll = ((offset[ROLL] + js_roll) < 0 ? 0 : offset[ROLL] + js_roll);
    // a_pitch = ((offset[PITCH] + js_pitch) < 0 ? 0 : offset[PITCH] + js_pitch);
    // a_yaw = ((offset[YAW] + js_yaw) < 0 ? 0 : offset[YAW] + js_yaw);
    // a_lift = ((offset[LIFT] + js_lift) < 0 ? 0 : offset[LIFT] + js_lift);

    oo1 = (a_lift + 2 * a_pitch - a_yaw);
	oo2 = (a_lift - 2 * a_roll + a_yaw);
	oo3 = (a_lift - 2 * a_pitch - a_yaw);
	oo4 = (a_lift + 2 * a_roll + a_yaw);

    oo1 = (oo1 < 200 ? MIN(a_lift, 200) : oo1);
    oo2 = (oo2 < 200? MIN(a_lift, 200) : oo2);
    oo3 = (oo3 < 200 ? MIN(a_lift, 200) : oo3);
    oo4 = (oo4 < 200? MIN(a_lift, 200) : oo4);

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