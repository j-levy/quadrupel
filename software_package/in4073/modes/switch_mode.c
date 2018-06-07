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
    mode_RUN[3] = mode_3_calib_RUN;
    mode_RUN[4] = mode_4_yaw_RUN;
    mode_RUN[5] = mode_5_full_RUN;
    mode_RUN[6] = mode_6_raw_RUN;


    mode_QUIT[0] = mode_0_safe_QUIT;
    mode_QUIT[1] = mode_1_panic_QUIT;
    mode_QUIT[2] = mode_2_manual_QUIT;
    mode_QUIT[3] = mode_3_calib_QUIT;
    mode_QUIT[4] = mode_4_yaw_QUIT;
    mode_QUIT[5] = mode_5_full_QUIT;
    mode_QUIT[6] = mode_6_raw_QUIT;

    mode_INIT[0] = mode_0_safe_INIT;
    mode_INIT[1] = mode_1_panic_INIT;
    mode_INIT[2] = mode_2_manual_INIT;
    mode_INIT[3] = mode_3_calib_INIT;
    mode_INIT[4] = mode_4_yaw_INIT;
    mode_INIT[5] = mode_5_full_INIT;
    mode_INIT[6] = mode_6_raw_INIT;


    /*TODO : add prototypes in header 
    */

    mode_CANENTER[0] = mode_0_safe_CANENTER;
    mode_CANENTER[1] = mode_1_panic_CANENTER;
    mode_CANENTER[2] = mode_2_manual_CANENTER;
    mode_CANENTER[3] = mode_3_calib_CANENTER;
    mode_CANENTER[4] = mode_4_yaw_CANENTER;
    mode_CANENTER[5] = mode_5_full_CANENTER;
    mode_CANENTER[6] = mode_6_raw_CANENTER;


    mode_CANLEAVE[0] = mode_0_safe_CANLEAVE;
    mode_CANLEAVE[1] = mode_1_panic_CANLEAVE;
    mode_CANLEAVE[2] = mode_2_manual_CANLEAVE;
    mode_CANLEAVE[3] = mode_3_calib_CANLEAVE;
    mode_CANLEAVE[4] = mode_4_yaw_CANLEAVE;
    mode_CANLEAVE[5] = mode_5_full_CANLEAVE;
    mode_CANLEAVE[6] = mode_6_raw_CANLEAVE;
}

void switch_mode( uint8_t newmode )
{

   if (newmode >= 0 && newmode <= 6)
   {
       if (mode_CANLEAVE[mode](newmode) && mode_CANENTER[newmode](mode))
       {
            mode_QUIT[mode]();
            mode = newmode;
            mode_INIT[mode]();
       }
   }

}