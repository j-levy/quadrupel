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
* Sampling frequency 1000Hz
* Cut off frequency 10Hz
*
* Jonathan Levy
*/
void filter_butter()
{
	xf[2] = xf[1];
	xf[1] = xf[0];
	xf[0] = (sp); // scaling to avoid overflow while keeping more precision on coeffs
	yf[2] = yf[1];
	yf[1] = yf[0];
	yf[0] = (xf[0]*af[0] + xf[1]*af[1] + xf[2]*af[2] + yf[1]*bf[1] + yf[2]*bf[2]) >> SHIFT;
	spf = yf[0]; 

}

void filter_k()
{
	// slideshow 5, slide 52, algorithm
	// i take the same constants on both directions
	// because symmetry.
	// I know real live doesn't work that way, but we won't be able to test it anyways.
	int32_t e;

	p_out = sp - p_b;
	phi_out = phi_out + p_out * P2PHI;
	e = phi_out - phi;
	phi_out = phi_out - e / C1;
	p_b = p_b + (e/P2PHI) / C2;

	q_out = sq - q_b;
	theta_out = theta_out + q_out * P2PHI;
	e = theta_out - theta;
	theta_out = theta_out - e / C1;
	q_b = q_b + (e/P2PHI) / C2;
	

	// I overwrite the variables.
	// In any case, whenever this function will be called again,
	// It will be because of the interrupt from the sensor
	// So these variables will be overwritten again by the sensor.
	phi = MAX(MIN((int16_t) phi_out, -32768), 32767);
	theta = MAX(MIN((int16_t) theta_out, -32768), 32767);
	sp = MAX(MIN((int16_t) p_out, -32768), 32767);
	sq = MAX(MIN((int16_t) q_out, -32768), 32767);

}

void run_filters_and_control()
{
	// fancy stuff here
	// control loops and/or filters

	// ae[0] = xxx, ae[1] = yyy etc etc

	int32_t oo[4];
    int32_t js[4];
    int32_t a[4];
    
    js[ROLL]= axis[ROLL] >> 2;
    js[PITCH] = axis[PITCH] >> 2;
    js[YAW] = axis[YAW] >> 2;
    js[LIFT] = (-(axis[LIFT] - 32767) >> 1);


	if (mode == 4)
	{
		js[YAW] = (p_yaw * (P_SCALE*js[YAW]/p_yaw + __SR))/P_SCALE;
		// you can add telemetry here!
	}

	if (mode == 5)
	{
		js[YAW] = ((p_yaw) * (P_SCALE*js[YAW]/(p_yaw) + __SR))/P_SCALE;

		js[ROLL]  = ((p_p1) * (P_SCALE*js[ROLL] /(p_p1) - __PHI ) - ((p_p2) * __SP))/P_SCALE;
		js[PITCH] = ((p_p1) * (P_SCALE*js[PITCH]/(p_p1) - __THETA) + ((p_p2) * __SQ))/P_SCALE;
		
		// you can add telemetry here! Be careful with the number of bytes though.
	}

	if (mode == 6)
	{
		//printf("RAW SENSORS |sax|%d|say|%d|saz|%d|sp|%d|sq|%d|sr|%d|\n",sax,say,saz,sp,sq,sr);
		filter_butter();
		//filter_k();
		// You can add telemetry herer!
			
		//Send telemtry packet for display on PC 	
		telemetry_packet[SRF] = MSBYTE(spf);
		telemetry_packet[SRF + 1] = LSBYTE(spf);
		telemetry_packet[SR] = MSBYTE(sp);
		telemetry_packet[SR + 1] = LSBYTE(sp);
	
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
        oo[i] = oo[i] >> 6; /// ( flight_coeffs[LIFT] + flight_coeffs[PITCH] + flight_coeffs[ROLL] + flight_coeffs[YAW]);  // sum(coefs) = 63, proche de 64

        // because oo_max = (sum_coeffs) * coeffs)*a_max
        // because coeff[ROLL] == coeff[PITCH] (I do this because of symmetry, makes sense)
		
        oo[i] = MAX(oo[i], 0);
        oo[i] = (oo[i] < MIN_SPEED*32 ? MIN(MAX(a[LIFT],0), MIN_SPEED*32) : oo[i]);
		oo[i] = (a[LIFT] < MIN_SPEED*32 ? MIN(MAX(a[LIFT],0), oo[i]) : oo[i]);
        oo[i] = MIN(oo[i], MAX_SPEED*32);
        ae[i] = oo[i] / (32); // scale 0->32767 to 0->1023, but capped to 600 anyway.
        
    }
	update_motors();
}
