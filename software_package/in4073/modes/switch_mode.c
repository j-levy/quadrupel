#include "in4073.h"
#include "switch_mode.h"



/*  
 Mode is a global variable, so it can be written and read right away.

Switch modes if the switching is legitimate

*/
void init_modes()
{
    mode_RUN[0] = mode_0_safe_RUN;
    mode_RUN[1] = mode_1_panic_RUN;
    mode_RUN[2] = mode_2_manual_RUN;


    mode_QUIT[0] = mode_0_safe_QUIT;
    mode_QUIT[1] = mode_1_panic_QUIT;
    mode_QUIT[2] = mode_2_manual_QUIT;


    mode_INIT[0] = mode_0_safe_QUIT;
    mode_INIT[1] = mode_1_panic_QUIT;
    mode_INIT[2] = mode_2_manual_QUIT;

    /*TODO : add prototypes in header 
    */

    mode_CANENTER[0] = mode_0_safe_CANENTER;
    mode_CANENTER[1] = mode_1_panic_CANENTER;
    mode_CANENTER[2] = mode_2_manual_CANENTER;

    mode_CANLEAVE[0] = mode_0_safe_CANLEAVE;
    mode_CANLEAVE[1] = mode_1_panic_CANLEAVE;
    mode_CANLEAVE[2] = mode_2_manual_CANLEAVE;
}

void switch_mode( uint8_t newmode )
{

    /*
    switch(mode)
    {
        case MODE_0_SAFE:
        if (newmode == MODE_2_MANUAL)
        {
            (*mode_QUIT[mode])();
            mode = newmode;
            (*mode_INIT[mode])();

        }
        break;
        case MODE_1_PANIC:
        if (newmode == MODE_0_SAFE)
        {
            (*mode_QUIT[mode])();
            mode = newmode;
            (*mode_INIT[mode])();

        }
        break;
        case MODE_2_MANUAL:
        if ((newmode == MODE_0_SAFE) || (newmode == MODE_1_PANIC) )
        {
            (*mode_QUIT[mode])();
            mode = newmode;
            (*mode_INIT[mode])();

        }
        break;
        
        default:
        printf("unknown mode\n");
        mode = MODE_1_PANIC; // if some weird shit happens, better be safe than sorry.
        break;

    }
    */
   if (newmode >= 0 && newmode <= 2)
   {
       if ((*mode_CANLEAVE[mode])(newmode) && (*mode_CANENTER[newmode])(mode))
       {
            (*mode_QUIT[mode])();
            mode = newmode;
            (*mode_INIT[mode])();
       }
   }

}