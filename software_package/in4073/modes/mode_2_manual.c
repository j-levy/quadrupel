#include "in4073.h"
#include "switch_mode.h"
#include "mode_2_manual.h"

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
    
    js_roll = axis[ROLL] / JS_SENSITIVITY;
    js_pitch = axis[PITCH] / JS_SENSITIVITY;

    // Yaw command is not cumulative in manual mode
    js_yaw = (axis[YAW] / JS_SENSITIVITY) * DT;
    js_lift = -(axis[LIFT] - 32767) / 2*JS_SENSITIVITY;

    a_roll = offset[ROLL] + js_roll;
    a_pitch = offset[PITCH] + js_pitch;
    a_yaw = offset[YAW] + js_yaw;
    a_lift = offset[LIFT] + js_lift;

    oo1 = (a_lift + 2 * a_pitch - a_yaw) / 4;
	oo2 = (a_lift - 2 * a_roll + a_yaw) / 4;
	oo3 = (a_lift - 2 * a_pitch - a_yaw) / 4;
	oo4 = (a_lift + 2 * a_roll + a_yaw) / 4;

	/* clip ooi as rotors only provide prositive thrust
	 */
	if (oo1 < 0) oo1 = 0;
	if (oo2 < 0) oo2 = 0;
	if (oo3 < 0) oo3 = 0;
	if (oo4 < 0) oo4 = 0;


	/* with ai = oi it follows
	 */
	ae[0] = sqrt(oo1)*SCALE;
	ae[1] = sqrt(oo2)*SCALE;
	ae[2] = sqrt(oo3)*SCALE;
	ae[3] = sqrt(oo4)*SCALE;



    update_motors();
}