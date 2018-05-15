#include "in4073.h"
#include "switch_mode.h"


/* 
 TODO: write the constants as #define
*/

static uint32_t time = 0;

void mode_1_panic_INIT()
{
    time = get_time_us(); // initialize the current time
    for (int i = 0; i < 4; i++)
        motor[i] = 350;
}

void mode_1_panic_QUIT()
{
    for (int i = 0; i < 4; i++)
        motor[i] = 0;
}

void mode_1_panic_RUN()
{    
    if (get_time_us() - time > 3000000) // wait for 3s
    {
        switch_mode(MODE_0_SAFE);
    }
}