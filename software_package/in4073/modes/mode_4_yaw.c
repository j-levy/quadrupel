/*
MODE 4: yaw control function.
Author:Yuhao Xuan
*/
#include "in4073.h"
#include "switch_mode.h"
#include "mode_4_yaw.h"
char mode_4_yaw_CANLEAVE(uint8_t target)
{
    char lock = 0;
    lock = (target != MODE_1_PANIC) && (target != MODE_0_SAFE);
    return (lock == 0);
}

char mode_2_yaw_CANENTER(uint8_t source)
{
    char lock = 0;

    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    lock = lock || buttons;
    
    return (lock == 0);
}
void mode_4_yaw_INIT()
{
// for now nothing to be done
}

void mode_4_yaw_QUIT()
{
// for now nothing to be done
}

void mode_4_yaw_RUN(void)

{
    int16_t oo1, oo2, oo3, oo4;
    int16_t js_roll, js_pitch, js_lift, js_yaw, a_roll, a_pitch, a_yaw, a_lift, r;
    //int16_t sax_state, say_state, saz_state;
    int16_t sr_state;
    int16_t a_roll_new, a_pitch_new, a_yaw_new, a_lift_new;
    //static int16_t js_yaw = 0;
    
    js_roll = axis[ROLL] / JS_SENSITIVITY;
    js_pitch = axis[PITCH] / JS_SENSITIVITY;
    js_yaw = axis[YAW] / JS_SENSITIVITY;
    js_lift = -(axis[LIFT] - 32767) / 2*JS_SENSITIVITY;
    //js_yaw = (axis[YAW] / JS_SENSITIVITY) * DT;



/*When implementing the control, the speed of the motor 
should not be 0, so that we can have a stable state with 
a speed??
*/
    a_roll = offset[ROLL] + js_roll;
    a_pitch = offset[PITCH] + js_pitch;
    a_yaw = offset[YAW] + js_yaw;
    a_lift = offset[LIFT] + js_lift;

    // yaw rate: r
    r = a_yaw;

    oo1 = (a_lift + 2 * a_pitch - a_yaw) / 4;
	oo2 = (a_lift - 2 * a_roll + a_yaw) / 4;
	oo3 = (a_lift - 2 * a_pitch - a_yaw) / 4;
	oo4 = (a_lift + 2 * a_roll + a_yaw) / 4;

	/* with ai = oi it follows
	 */
	ae[0] = sqrt(oo1)*SCALE;
	ae[1] = sqrt(oo2)*SCALE;
	ae[2] = sqrt(oo3)*SCALE;
	ae[3] = sqrt(oo4)*SCALE;

	/* clip ooi as rotors only provide prositive thrust
	 */
    if (oo1 < 0) oo1 = 0;
	if (oo2 < 0) oo2 = 0;
	if (oo3 < 0) oo3 = 0;
	if (oo4 < 0) oo4 = 0;
    update_motors();

    //read sensor data
    sr_state = sr;

    /*Kalman Filtering could be implemented later.*/

    //implement P control(a_lift_new, a_roll_new and a_pitch_new are left for full control)
    while(sr_state != r){ 
    a_yaw_new = 5 * (sr_state - r);
    a_lift_new = a_lift;
    a_roll_new = a_roll;
    a_pitch_new = a_pitch;

    oo1 = (a_lift_new + 2 * a_pitch_new - a_yaw_new) / 4;
	oo2 = (a_lift_new - 2 * a_roll_new + a_yaw_new) / 4;
	oo3 = (a_lift_new - 2 * a_pitch_new - a_yaw_new) / 4;
	oo4 = (a_lift_new + 2 * a_roll_new + a_yaw_new) / 4;

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

    update_motors();}
   
}