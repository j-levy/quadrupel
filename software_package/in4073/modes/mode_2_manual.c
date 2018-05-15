#include "in4073.h"
#include "switch_mode.h"
#include "mode_2_manual.h"

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
    js_yaw += (axis[YAW] / JS_SENSITIVITY) * DT;
    js_lift = -(axis[LIFT] - 32767) / JS_SENSITIVITY;

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
	ae[0] = sqrt(oo1);
	ae[1] = sqrt(oo2);
	ae[2] = sqrt(oo3);
	ae[3] = sqrt(oo4);

    for (int i = 0; i < 4; i++)
    {
        ae[i] = (ae[i] > 500) ? 500 : ae[i];
    }

    update_motors();
}