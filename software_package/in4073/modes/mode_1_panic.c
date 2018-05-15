#include "in4073.h"
#include "switch_mode.h"


/* 
 TODO: write the constants as #define
*/

char mode_1_panic_CANLEAVE(uint8_t target)
{
    // check if no input is provided from keyboard AND joystick
    char lock = 0;
    lock |= (axis[ROLL] != 0);
    lock |= (axis[PITCH] != 0);
    lock |= (axis[YAW] != 0);
    lock |= ((axis[LIFT]/2+16384) != 0);

    lock |= buttons;
    lock |= (keyboard_key == 0);

    lock |= (target != MODE_0_SAFE);
    
    // if one of these things is set to 1, res is set to 1.
    return (1-lock);
}

char mode_1_panic_CANENTER(uint8_t source)
{
    return 1;
}

void mode_1_panic_INIT()
{
    for (int i = 0; i < 4; i++)
        motor[i] = 100;
}

void mode_1_panic_QUIT()
{
    for (int i = 0; i < 4; i++)
        motor[i] = 0;
}

void mode_1_panic_RUN()
{    
    if (motor[1] + motor[2] + motor[3] + motor[0] == 0)
    {
        switch_mode(MODE_0_SAFE);
    }
}