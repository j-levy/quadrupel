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
 *------------------------------------------------------------------
 */

#include "in4073.h"

#define DEBUG	



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
}

void store_joystick_button(uint8_t *val)
{
	buttons = *val;
}


void store_key(uint8_t *val)
{
	keyboard_key = *val;
}

void store_mode(uint8_t *val)
{
	nextmode = *val - '0';
}



/*------------------------------------------------------------------
 * main -- everything you need is here :)
 *------------------------------------------------------------------
 */
int main(void)
{
	uart_init();
	gpio_init();
	timers_init();
	adc_init();
	twi_init();
	imu_init(true, 100);	
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

	while (!demo_done)
	{
		if (rx_queue.count)
		{
			process_packet( dequeue(&rx_queue) );
		}

		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) 
				nrf_gpio_pin_toggle(BLUE);

			adc_request_sample();

			read_baro();

			
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

			
			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			run_filters_and_control();
		}


		
		if (nextmode != mode)
			switch_mode(nextmode);
		
		
		
		mode_RUN[mode]();
		

	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
