/*------------------------------------------------------------------
 *  in4073.c -- test QR engines and sensors
 *
 *  reads ae[0-3] uart rx queue
 *  (q,w,e,r increment, a,s,d,f decrement)
 *
 *  prints timestamp, ae[0-3], sensors to uart tx queue
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  June 2016
 * 
 * Revised May 2018
 * Author: Niket Agrawal
 *------------------------------------------------------------------
 */

#include "in4073.h"

// #define DEBUG_TIMEOUT
//#define BATTERY_MONITORING	

uint8_t buttons = 0;
int16_t axis[4] = {0};
uint8_t keyboard_key = 0; 
uint8_t mode = 0;
uint8_t abort_mission = 0;


/*------------------------------------------------------------------
 * process_{joystick, key} -- process command keys, mode change, or joystick
 * Jonathan Lévy
 * April 2018
 *------------------------------------------------------------------
 */

uint8_t nextmode;

void store_joystick_axis(uint8_t *val)
{
	for (int i = 0; i < 4; i++)
	{
		int16_t stickvalue = (((int16_t) *(val + 2*i)) << 8) + ((int16_t) *(val + 2*i + 1));
		axis[i] = stickvalue;
	}
	
	//If throttle is almost zero, clear the offset
	if(axis[3] > 32000)
	{
		for(int i =0; i<4; i++)
        {
            offset[i] = 0;
        }
	}		
}

void store_joystick_button(uint8_t *val)
{
	buttons = *val;
}

void store_key(uint8_t *val)
{
	keyboard_key = *val;
	switch(keyboard_key)
	{
		case 'u': proportional_controller_yaw += P_SCALING;
				  break;
		case 'j': proportional_controller_yaw = (proportional_controller_yaw > P_SCALING ? proportional_controller_yaw-P_SCALING : 1);
			      break;

		case 'a': offset[LIFT] += OFFSET_SCALING; //lift up
				  break;

		case 'z': offset[LIFT] -= OFFSET_SCALING;
					//offset[LIFT] = (offset[LIFT] > OFFSET_SCALING ? offset[LIFT] - OFFSET_SCALING : 0); //lift down
				  break;	

		case 44:  offset[PITCH] += OFFSET_SCALING; //pitch up
				  break;

		case 42:  offset[PITCH] -= OFFSET_SCALING ; //pitch down
				  break;

		case 43:  offset[ROLL] += OFFSET_SCALING; //roll down
				  break;

		case 45:  offset[ROLL] -= OFFSET_SCALING ; //roll up
				  break;

		case 'w': offset[YAW] += OFFSET_SCALING; //yaw up
				  break; 

		case 'q': offset[YAW] -= OFFSET_SCALING ; //yaw down
				  break;	

	}
	telemetry_packet[P_VALUE] = proportional_controller_yaw;
}

void store_mode(uint8_t *val)
{
	if (*val == 27)
	{
		abort_mission = 1;
		nextmode = 1;
	}
	else
		nextmode = *val - '0';
}


/*------------------------------------------------------------------
 * main -- everything you need is here :)
 *------------------------------------------------------------------
 */
int main(void)
{
	is_DMP_on = true;
	is_calibration_done = false;

	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	imu_init(is_DMP_on, 100);	
	baro_init();
	spi_flash_init();
	ble_init();

	init_modes();

	uint32_t counter = 0;
	demo_done = false;
	mode = 0;

	nextmode = 0;


	buttons = 0;
	keyboard_key = 0;

	for (int i = 0; i < 4; i++)
		axis[i] = 0;

	uint32_t tx_timer = 0;
	uint32_t delta_time = 0;
	uint32_t timeout = 0;
	#ifdef DEBUG_TIMEOUT
	uint32_t count = 0; // for timeout testing purpose
	#endif

	while (!demo_done)
	{
		//Enter panic mode if battery is low
		//and if not in safe or panic mode already
		#ifdef BATTERY_MONITORING
		if((bat_volt < BATTERY_THRESHOLD) && (mode && (mode != 1)))
			nextmode = 1;
		#endif

		#ifdef DEBUG_TIMEOUT
		if(count%5000 == 0) //timeout failure scenario testcase. happens multiple time this way.
		{
			delta_time = 160000;
		}
		else
		#endif
			delta_time = get_time_us() - timeout;

		if(timeout && (delta_time > RX_TIMEOUT) && (delta_time > 0))
		{
			//printf("%10ld %10ld %10ld \n", get_time_us(), timeout, delta_time); 
			printf("comm link failure\n");
			comm_link_failure = 1; 
			nrf_gpio_pin_toggle(RED);

			if(mode && (mode != 1))		//Enter panic mode only if NOT 
										//in safe or panic mode already
			{
				nextmode = 	1; 
			}		
				
		}

		if (rx_queue.count)
		{
			#ifdef DEBUG_TIMEOUT
			count++;
			#endif
			timeout = get_time_us();
			process_packet( dequeue(&rx_queue) );
		}

		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) 
				nrf_gpio_pin_toggle(BLUE);

			adc_request_sample();

			read_baro();

			/*
			#ifdef DEBUG

				printf("%10ld | ", get_time_us());
				printf("ae: %3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
				printf("motor: %3d %3d %3d %3d | ",motor[0],motor[1],motor[2],motor[3]);
				//printf("%6d %6d %6d | ", phi, theta, psi);
				//printf("%6d %6d %6d | ", sp, sq, sr);
				//printf("%4d | %4ld | %6ld | ", bat_volt, temperature, pressure);
				
				printf("axis: %d %d %d %d |", axis[0], axis[1], axis[2], ((-(axis[LIFT] - 32767) / 2)));

				printf("%d |", buttons);
 
				printf("%d | ", mode);
				printf("\n");

			#endif
			*/

			//Filling rotor RPM data
			for (int j = 0; j < 4; j++)
			{
				telemetry_packet[ROTOR1 + 2*j] = MSBYTE( ae[j] );
				telemetry_packet[ROTOR1 + 2*j + 1] = LSBYTE( ae[j] );
			}	

			//Mode
			telemetry_packet[MODE_DRONE] = mode;

			//Attitude
			// telemetry_packet[PHI] = MSBYTE(phi);
			// telemetry_packet[PHI + 1] = LSBYTE(phi);
			// telemetry_packet[THETA] = MSBYTE(theta);
			// telemetry_packet[THETA + 1] = LSBYTE(theta);
			// telemetry_packet[PSI] = MSBYTE(psi);
			// telemetry_packet[PSI + 1] = LSBYTE(psi);

			//Angular velocity
			// telemetry_packet[SP] = MSBYTE(sp);
			// telemetry_packet[SP + 1] = LSBYTE(sp);
			// telemetry_packet[SQ] = MSBYTE(sq);
			// telemetry_packet[SQ + 1] = LSBYTE(sq);
			// telemetry_packet[SR] = MSBYTE(sr);
			// telemetry_packet[SR + 1] = LSBYTE(sr);
			
			telemetry_packet[BAT_VOLT] = MSBYTE(bat_volt);
			telemetry_packet[BAT_VOLT + 1] = LSBYTE(bat_volt);

			// telemetry_packet[TEMPERATURE] = MSBYTE_WORD(temperature);
			// telemetry_packet[TEMPERATURE + 1] = BYTE2_WORD(temperature);
			// telemetry_packet[TEMPERATURE + 2] = BYTE3_WORD(temperature);
			// telemetry_packet[TEMPERATURE + 3] = LSBYTE_WORD(temperature);

			// telemetry_packet[PRESSURE] = MSBYTE_WORD(pressure);
			// telemetry_packet[PRESSURE + 1] = BYTE2_WORD(pressure);
			// telemetry_packet[PRESSURE + 2] = BYTE3_WORD(pressure);
			// telemetry_packet[PRESSURE + 3] = LSBYTE_WORD(pressure);
			
			clear_timer_flag();
		}

		// if (nextmode != mode)
		// 	switch_mode(nextmode); //commenting this additional code which possible got pasted twice in master


		// Note: this is probably something that will be included in the mode functions.
		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			run_filters_and_control();
		}
		
		

		if (nextmode != mode)
			switch_mode(nextmode);

		
		/*For the sequence Esc -> panic > safe > abort
		//if(!mode && abort_mission)
			//demo_done = true;
		//else
		// ADDENDUM: this part is in MODE_0_SAFE_RUN
		*/
			mode_RUN[mode]();
		
		if ((get_time_us() - tx_timer) > TELEMETRY_TX_INTERVAL)
		{
			tx_timer = get_time_us();
			send_telemetry_packet();
		}
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
