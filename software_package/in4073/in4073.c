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
#include "modes/switch_mode.h"

#define MIN(a,b) (a < b ? a : b)

#define MAX(a,b) (a > b ? a : b)

// #define DEBUG_TIMEOUT
//#define BATTERY_MONITORING	

uint8_t buttons = 0;
int16_t axis[4] = {0};
uint8_t keyboard_key = 0; 
uint8_t mode = 0;
uint8_t abort_mission = 0;

uint8_t nextmode;

/*------------------------------------------------------------------
 * process joystick axis values
 * Jonathan Lévy
 * April 2018
 *------------------------------------------------------------------
 */

void store_joystick_axis(uint8_t *val)
{
	for (int i = 0; i < 4; i++)
	{
		int16_t stickvalue = (((int16_t) *(val + 2*i)) << 8) + ((int16_t) *(val + 2*i + 1));
		axis[i] = stickvalue;
	}
}

/*------------------------------------------------------------------
 * process joystick button input and handling abort mission
 * Niket Agrawal
 * May 2018
 *------------------------------------------------------------------
 */
void store_joystick_button(uint8_t *val)
{
	buttons = *val;
	//Enter panic mode if joystick fire button (Abort mission) pressed
	if(buttons == 1)
	{
		abort_mission = 1;
		nextmode = 1;
	}
		
}

/*------------------------------------------------------------------
 * process joystick key, handle static trimming offset,
 * handle modification of control loop constants
 * Author - Niket Agrawal
 * Flight coefficients - Jonathan Lévy
 * May 2018
 *------------------------------------------------------------------
 */

void store_key(uint8_t *val)
{
	keyboard_key = *val;
	switch(keyboard_key)
	{
    /* Keys for P controllers adjust*/
		case 'u': p_yaw = MIN(p_yaw+1, 65534);
				  break;
		case 'j': p_yaw = MAX(p_yaw-1, 1);
			      break;
		case 'i': p_p1 = MIN(p_p1+1, 65534);
				  break;
		case 'k': p_p1 = MAX(p_p1-1, 1);
			      break;
		case 'o': p_p2 = MIN(p_p2+1, 65534);
				  break;	
		case 'l': p_p2 = MAX(p_p2-1, 1);
			      break;
      
      
    /* keys for relative coefficient for flight adjust */
		case 't' : flight_coeffs[LIFT]++; break;
		case 'g' : flight_coeffs[LIFT] = MAX(flight_coeffs[LIFT]-1, 1); break;
		case 'y' : flight_coeffs[ROLL]++; 
					flight_coeffs[PITCH] = flight_coeffs[ROLL];break;
		case 'h' : flight_coeffs[ROLL] = MAX(flight_coeffs[ROLL]-1, 1); 
					flight_coeffs[PITCH] = flight_coeffs[ROLL]; break;
		case 'r' : flight_coeffs[YAW]++; break;
		case 'f' : flight_coeffs[YAW] = MAX(flight_coeffs[YAW]-1, 1); break;
      
     /* keys for offset adjust */
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
		case 'a': offset[LIFT] += OFFSET_SCALING; //yaw up
				  break; 
		case 'z': offset[LIFT] -= OFFSET_SCALING ; //yaw down
				  break;
	}
	
	telemetry_packet[P_YAW] = MSBYTE(p_yaw);
	telemetry_packet[P_YAW+1] = LSBYTE(p_yaw);
	telemetry_packet[P1] = MSBYTE(p_p1);
	telemetry_packet[P1+1] = LSBYTE(p_p1);
	telemetry_packet[P2] = MSBYTE(p_p2);
	telemetry_packet[P2+1] = LSBYTE(p_p2);
}

/*------------------------------------------------------------------
 * process mode information from keyboard
 * Niket Agrawal
 * May 2018
 *------------------------------------------------------------------
 */
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
 * main -- everything you need is here 
 * Various contributions
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
	//log_init();

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


			//Filling rotor RPM data
			for (int j = 0; j < 4; j++)
			{
				telemetry_packet[ROTOR1 + 2*j] = MSBYTE( ae[j] );
				telemetry_packet[ROTOR1 + 2*j + 1] = LSBYTE( ae[j] );
			}	

			//Mode & battery voltage
			telemetry_packet[MODE_DRONE] = mode;
			telemetry_packet[BAT_VOLT] = MSBYTE(bat_volt);
			telemetry_packet[BAT_VOLT + 1] = LSBYTE(bat_volt);

			clear_timer_flag();
		}
    
    	if (check_sensor_int_flag()) 
		{
			get_dmp_data();

			if (mode == MODE_2_MANUAL || mode == MODE_4_YAWCTRL || mode == MODE_5_FULLCTRL )
				run_filters_and_control();
		}
		
		

		if (nextmode != mode)
			switch_mode(nextmode);

		
		mode_RUN[mode]();
		
		if ((get_time_us() - tx_timer) > TELEMETRY_TX_INTERVAL)
		{
			tx_timer = get_time_us();
			send_telemetry_packet();
			//log_sensor();
		}
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
