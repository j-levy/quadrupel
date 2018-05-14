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
        case MODESAFE:
        if (MOTORS_OFF && (newmode == MODECALIB || newmode == MODEMANUAL))
            mode = newmode;
        break:
        case MODEPANIC;
        if (MOTORS_OFF && (newmode == MODESAFE))
            mode = newmode;
        break;
        case MODEMANUAL:
        if ((newmode == MODESAFE) || (newmode == MODEPANIC) )
            mode = newmode;
        break;


        
        default:
        printf("unknown mode\n");
        break;
    }

    return true;
}