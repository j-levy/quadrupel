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

/*------------------------------------------------------------------
 * process_{joystick, key} -- process command keys, mode change, or joystick
 * Jonathan Lévy
 * April 2018
 *------------------------------------------------------------------
 */
void process_joystick_axis(uint8_t *val)
{
	printf("\n============== Stick values:");	
	for (int i = 0; i < 4; i++)
	{
		int16_t stickvalue = (((int16_t) *(val + 2*i)) << 8) + ((int16_t) *(val + 2*i + 1));
		printf("%d, ",stickvalue);
	}
}

void process_joystick_button(uint8_t *val)
{
	for (int i = 0; i < 8; i++)
	{
		if ((*val & (1<<i)) >> i) // (if button #i has been pressed)
		{	
			printf("\n=============== BUTTON %d PRESSED ==============\n", i);
		}
	}
}

void process_key(uint8_t *val)
{
	if (*val >= '0' && *val <= '8')
	{
		mode = *val;
		printf("=============\n\t\tMode changed to %d\n", *val);
	} else {
		switch (*val)
		{
			case 'q':
				ae[0] += 10;
				break;
			case 'a':
				ae[0] -= 10;
				if (ae[0] < 0) ae[0] = 0;
				break;
			case 'w':
				ae[1] += 10;
				break;
			case 's':
				ae[1] -= 10;
				if (ae[1] < 0) ae[1] = 0;
				break;
			case 'e':
				ae[2] += 10;
				break;
			case 'd':
				ae[2] -= 10;
				if (ae[2] < 0) ae[2] = 0;
				break;
			case 'r':
				ae[3] += 10;
				break;
			case 'f':
				ae[3] -= 10;
				if (ae[3] < 0) ae[3] = 0;
				break;
			case 27:
				demo_done = true;
				break;
			default:
				nrf_gpio_pin_toggle(RED);
		}
	}
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

	uint32_t counter = 0;
	demo_done = false;
	mode = 0;

	while (!demo_done)
	{
		if (rx_queue.count) 
			process_packet( dequeue(&rx_queue) );

		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) 
				nrf_gpio_pin_toggle(BLUE);

			adc_request_sample();
			read_baro();

			/*

			printf("%10ld | ", get_time_us());
			printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
			
			printf("%6d %6d %6d | ", phi, theta, psi);
			printf("%6d %6d %6d | ", sp, sq, sr);
			printf("%4d | %4ld | %6ld \n", bat_volt, temperature, pressure);
			

			printf("%d | ", mode);
			printf("\n");

			*/
			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			run_filters_and_control();
		}
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
