#include "in4073.h"
#include "switch_mode.h"
#include "modes_flight.h"
#include "fixed_point.h"


char mode_6_raw_CANLEAVE(uint8_t target)
{
    char lock = 0;
    lock = (target != MODE_1_PANIC) && (target != MODE_0_SAFE);
    return (lock == 0);
}

char mode_6_raw_CANENTER(uint8_t source)
{
    char lock = 0;

    lock = (axis[ROLL] != 0) || (axis[PITCH] != 0) || (axis[YAW] != 0) || ((-(axis[LIFT] - 32767) / 2) != 0);

    lock = lock || buttons;
    
    return (lock == 0);
}
void mode_6_raw_INIT()
{
    //initialize the imu to raw mode
    imu_init(false, 1000);
    //printf("MODE 6 ENTERED");


    // initialize the global variable for the controller.
    for (int i = 0; i < 4; i++) 
        flight_coeffs[i] = 1;
    
    for(int j=0; j < 3; j++)
    {
        xf[j] = 0;
        yf[j] = 0;
    }

    // b[0] = TOFIXEDPOINT(0.0675);
    // b[1] = TOFIXEDPOINT(0.01349);
    // b[2] = TOFIXEDPOINT(0.0675);
    // a[0] = TOFIXEDPOINT(1.0000);
    // a[1] = TOFIXEDPOINT(-1.1430);
    // a[2] = TOFIXEDPOINT(0.4128);

    bf[0] = 0.0675 * (1 << SHIFT);
    bf[1] = 0.01349 * (1 << SHIFT);
    bf[2] = 0.0675 * (1 << SHIFT);
    af[0] = 1.0000 * (1 << SHIFT);
    af[1] = -1.1430 * (1 << SHIFT);
    af[2] = 0.4128 * (1 << SHIFT);
    
    // coefficients determined empirically. They seem more or less ok.
    flight_coeffs[ROLL] = 9;
    flight_coeffs[PITCH] = flight_coeffs[ROLL];
    flight_coeffs[LIFT] = 3*flight_coeffs[ROLL];
    flight_coeffs[YAW] = 2*flight_coeffs[ROLL];

    // initialize the global variable for the controller.
    p_yaw = P_SCALE;
    p_p1 = P_SCALE;
    p_p2 = P_SCALE;

    //filter_butter();
    //calibration();
}

void mode_6_raw_QUIT()
{
// for now nothing to be done
}

void mode_6_raw_RUN(void)

{
    // code deported - not to be run at crazy speed
}