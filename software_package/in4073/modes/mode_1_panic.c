#include "in4073.h"
#include "switch_mode.h"
#include "modes_flight.h"


/*
Panic functions for the state machine.
Jonathan Lévy
*/


/* 
 TODO: write the constants as #define
*/
static uint16_t counter;

char mode_1_panic_CANLEAVE(uint8_t target)
{
    // check if no input is provided from keyboard AND joystick
    char lock = 0;
    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    lock = lock || buttons;
    lock = lock || (target != MODE_0_SAFE);
    
    // if one of these things is set to 1, res is set to 1.
    return (lock == 0);
}

char mode_1_panic_CANENTER(uint8_t source)
{
    return 1;
}

void mode_1_panic_INIT()
{
    for (int i = 0; i < 4; i++)
    {
        ae[i] = MIN(ae[i], MIN_SPEED);
        offset[i] = 0;
    }
    counter = 0;
    update_motors();
}

void mode_1_panic_QUIT()
{
    for (int i = 0; i < 4; i++)
        ae[i] = 0;

    update_motors();
}

void mode_1_panic_RUN()
{    
    counter++;
    if (counter%500 == 0) // slow down a bit
    {
        for (int i = 0; i < 4; i++)
            ae[i] = (ae[i] - 1 < 0 ? 0 : ae[i] - 1);
        
        update_motors();
        if ((motor[0] == 0) && (motor[1] == 0)  && (motor[2] == 0) && (motor[3] == 0))
        {
            switch_mode(MODE_0_SAFE);
        }
    }

}