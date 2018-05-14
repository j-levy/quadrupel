#include "in4073.h"
#include "switch_mode.h"




/*  
 Mode is a global variable, so it can be written and read right away.

Switch modes if the switching is legitimate

*/
bool switch_mode( uint8_t newmode )
{
    switch(mode)
    {
        case MODE_0_SAFE:
        if (MOTORS_OFF && (newmode == MODE_3_CALIB || newmode == MODE_2_MANUAL))
            mode = newmode;
        break:
        case MODE_1_PANIC;
        if (MOTORS_OFF && (newmode == MODE_0_SAFE))
            mode = newmode;
        break;
        case MODE_2_MANUAL:
        if ((newmode == MODE_0_SAFE) || (newmode == MODE_1_PANIC) )
            mode = newmode;
        break;


        
        default:
        printf("unknown mode\n");
        mode = MODE_1_PANIC; // if some weird shit happens, better be safe than sorry.
        break;



    }

    return true;
}