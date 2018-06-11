/*------------------------------------------------------------------
 *  control.c -- here you can implement your control algorithm
 *		 and any motor clipping or whatever else
 *		 remember! motor input =  0-1000 : 125-250 us (OneShot125)
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
#include "modes/modes_flight.h"
#include "fixed_point.h"

void update_motors(void)
{					
	
	motor[0] = ae[0];
	motor[1] = ae[1];
	motor[2] = ae[2];
	motor[3] = ae[3];
}

/*
* 2nd order Butterworth filter
* Sampling frequency 100Hz
* Cut off frequency 10Hz
*
* Niket Agrawal
*/
void filter_butter()
{
	xf[2] = sr;
	
	yf[2] = fixdiv(((fixmulint(af[0], xf[2]) + fixmulint(af[1], xf[1]) + 
	fixmulint(af[2], xf[0]) - fixmulint(bf[1], yf[1]) - fixmulint(bf[2], yf[0]))
	* (1 << 14)), bf[0]);
	xf[0] = xf[1];
	yf[0] = yf[1];
	xf[1] = xf[2];
	yf[1] = yf[2];
	srf = yf[2];

	//Send telemtry packet for display on PC 	
	telemetry_packet[SRF] = MSBYTE(srf);
	telemetry_packet[SRF + 1] = LSBYTE(srf);
	telemetry_packet[SR] = MSBYTE(sr);
	telemetry_packet[SR + 1] = LSBYTE(sr);
}

void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc

	int32_t oo[4];
    int32_t js[4];
    int32_t a[4];
    
    js[ROLL]= axis[ROLL] ;
    js[PITCH] = axis[PITCH] ;
    js[YAW] = axis[YAW] ;
    js[LIFT] = (-(axis[LIFT] - 32767) >> 1);


	if (mode == 4)
	{
		js[YAW] = (p_yaw * (P_SCALE*js[YAW]/p_yaw - __SR))/P_SCALE;
		// you can add telemetry here!
	}

	if (mode == 5)
	{
		js[YAW] = ((p_yaw) * (P_SCALE*js[YAW]/(p_yaw) - __SR))/P_SCALE;

		js[ROLL]  = ((p_p1) * (P_SCALE*js[ROLL] /(p_p1) - __PHI ) - ((p_p2) * __SP))/P_SCALE;
		// change sign of p_p2 ?
		js[PITCH] = ((p_p1) * (P_SCALE*js[PITCH]/(p_p1) - __THETA) + ((p_p2) * __SQ))/P_SCALE;
		
		// <divide by 8
		// you can add telemetry here! Be careful with the number of bytes though.
	}

	if (mode == 6)
	{
		//printf("RAW SENSORS |sax|%d|say|%d|saz|%d|sp|%d|sq|%d|sr|%d|\n",sax,say,saz,sp,sq,sr);
		filter_butter();
	
	}

    for (int i = 0; i < 4; i++)
    {
        a[i] = offset[i] + js[i];
    }

    oo[0] = a[LIFT]*flight_coeffs[LIFT] + 2 * a[PITCH]*flight_coeffs[PITCH] - a[YAW]*flight_coeffs[YAW] ;
	oo[1] = a[LIFT]*flight_coeffs[LIFT] - 2 * a[ROLL]*flight_coeffs[ROLL] + a[YAW]*flight_coeffs[YAW];
	oo[2] = a[LIFT]*flight_coeffs[LIFT] - 2 * a[PITCH]*flight_coeffs[PITCH] - a[YAW]*flight_coeffs[YAW] ;
	oo[3] = a[LIFT]*flight_coeffs[LIFT] + 2 * a[ROLL]*flight_coeffs[ROLL] + a[YAW]*flight_coeffs[YAW] ;

    for (int i = 0; i < 4 ; i++)
    {
        oo[i] = oo[i] / ( flight_coeffs[LIFT] + flight_coeffs[PITCH] + flight_coeffs[ROLL] + flight_coeffs[YAW]); 
        // because oo_max = (sum_coeffs) * coeffs)*a_max
        // because coeff[ROLL] == coeff[PITCH] (I do this because of symmetry, makes sense)
        oo[i] = MAX(oo[i], 0);
        oo[i] = (oo[i] < MIN_SPEED*32 ? MIN(a[LIFT], MIN_SPEED*32) : oo[i]);
		oo[i] = (a[LIFT] < MIN_SPEED*32 ? MIN(a[LIFT], oo[i]) : oo[i]);
        oo[i] = MIN(oo[i], MAX_SPEED*32);
        ae[i] = oo[i] / (32); // scale 0->32767 to 0->1023, but capped to 600 anyway.
        
    }
	update_motors();
}
