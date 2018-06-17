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
    //initialize the imu to raw mode at 1000 Hz
    imu_init(false, 1024);


    // coefficients determined empirically. They seem more or less ok.
    flight_coeffs[ROLL] = 9;
    flight_coeffs[PITCH] = flight_coeffs[ROLL];
    flight_coeffs[LIFT] = 3*flight_coeffs[ROLL];
    flight_coeffs[YAW] = 2*flight_coeffs[ROLL];
    

    // credits: http://www-users.cs.york.ac.uk/~fisher/mkfilter/
    /* sampling 1000Hz, cutoff 10Hz*/
    /*
    float gain = 1058.546241;
    int32_t fp_gain = 1 << SHIFT;

    float af_f[3];
    float bf_f[3];
    af_f[0] = (1/gain);
    af_f[1] = (2/gain);
    af_f[2] = (1/gain);
    bf_f[0] = 0;
    bf_f[1] = 1.9111970674;
    bf_f[2] =  -0.9149758348;
    */


    /* samping 1000Hz, cutoff 2Hz, ORDER 1
        Lowered order to have lower cutoff with better precision (lower numbers) */
    
    float gain = 1.601528487e+02;
    int32_t fp_gain = 1 << SHIFT;

    float af_f[3];
    float bf_f[3];
    af_f[0] = (1/gain);
    af_f[1] = (1/gain);
    af_f[2] = 0;
    bf_f[0] = 0; // unused
    bf_f[1] = 0.9875119299;
    bf_f[2] =  0;
    

    

    for(int j=0; j < 3; j++)
    {
        xf[j] = 0;
        yf[j] = 0;
        bf[j] = (int32_t) (bf_f[j] * fp_gain);
        af[j] = (int32_t) (af_f[j] * fp_gain);
    }
    // initialize the global variable for the controller.
    p_yaw = P_SCALE;
    p_p1 = P_SCALE;
    p_p2 = P_SCALE;

    C1 = P_SCALE; // small=believe sphi, large=believe sp
    C2 = P_SCALE * 1000; // >1000*C1: slow drift
    P2PHI = P_SCALE; // depends on loop freq: measure/compute

    //filter_butter();
}

void mode_6_raw_QUIT()
{
    imu_init(true,100); // turn back the DMP on
    // for now nothing to be done
}

void mode_6_raw_RUN(void)
{
    // code deported - not to be run at crazy speed
}