#include "in4073.h"
#include "switch_mode.h"



/*  
    Switch mode functions and actual mode-switcher
    Jonathan LEVY

    The state machine is created by each mode having 5 functions,
    that can be accessed with function pointers.
    These 5 types of functions are:
        - Init (run when entering the mode)
        - Quit (run when exiting the mode)
        - Can Enter (returns 1 if the mode can be left, 0 otherwise)
        - Can Leave (returns 1 if the mode can be entered, 0 otherwise)
        - Run (is run when in the mode. This is useful for Panic and Calibration, the flight code being in control.c)
*/
void init_modes()
{
    mode_RUN[0] = mode_0_safe_RUN;
    mode_RUN[1] = mode_1_panic_RUN;
    mode_RUN[2] = mode_2_manual_RUN;
    mode_RUN[3] = mode_3_calib_RUN;
    mode_RUN[4] = mode_4_yaw_RUN;
    mode_RUN[5] = mode_5_full_RUN;


    mode_QUIT[0] = mode_0_safe_QUIT;
    mode_QUIT[1] = mode_1_panic_QUIT;
    mode_QUIT[2] = mode_2_manual_QUIT;
    mode_QUIT[3] = mode_3_calib_QUIT;
    mode_QUIT[4] = mode_4_yaw_QUIT;
    mode_QUIT[5] = mode_5_full_QUIT;

    mode_INIT[0] = mode_0_safe_INIT;
    mode_INIT[1] = mode_1_panic_INIT;
    mode_INIT[2] = mode_2_manual_INIT;
    mode_INIT[3] = mode_3_calib_INIT;
    mode_INIT[4] = mode_4_yaw_INIT;
    mode_INIT[5] = mode_5_full_INIT;

    mode_CANENTER[0] = mode_0_safe_CANENTER;
    mode_CANENTER[1] = mode_1_panic_CANENTER;
    mode_CANENTER[2] = mode_2_manual_CANENTER;
    mode_CANENTER[3] = mode_3_calib_CANENTER;
    mode_CANENTER[4] = mode_4_yaw_CANENTER;
    mode_CANENTER[5] = mode_5_full_CANENTER;


    mode_CANLEAVE[0] = mode_0_safe_CANLEAVE;
    mode_CANLEAVE[1] = mode_1_panic_CANLEAVE;
    mode_CANLEAVE[2] = mode_2_manual_CANLEAVE;
    mode_CANLEAVE[3] = mode_3_calib_CANLEAVE;
    mode_CANLEAVE[4] = mode_4_yaw_CANLEAVE;
    mode_CANLEAVE[5] = mode_5_full_CANLEAVE;
}


void switch_mode( uint8_t newmode )
{

   if (newmode >= 0 && newmode <= 5)
   {
       if (mode_CANLEAVE[mode](newmode) && mode_CANENTER[newmode](mode))
       {
            mode_QUIT[mode]();
            mode = newmode;
            mode_INIT[mode]();
       }
   }

}