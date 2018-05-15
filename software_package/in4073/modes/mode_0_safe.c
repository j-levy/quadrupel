#include "in4073.h"
#include "switch_mode.h"

char mode_0_safe_CANLEAVE(uint8_t target)
{
    char lock = 0;

    lock |= (axis[ROLL] != 0);
    lock |= (axis[PITCH] != 0);
    lock |= (axis[YAW] != 0);
    lock |= ((axis[LIFT]/2+16384) != 0);

    lock |= buttons;
    lock |= (target != MODE_2_MANUAL);
    
    // if one of these things is set to 1, res is set to 1.
    return (1-lock);
}

char mode_0_safe_CANENTER(uint8_t source)
{
    char lock = 0;

    lock |= (motor[0] != 0);
    lock |= (motor[1] != 0);
    lock |= (motor[2] != 0);
    lock |= (motor[3] != 0);

    return (1-lock);
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