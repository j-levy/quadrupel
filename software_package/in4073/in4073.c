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

//#define DEBUG	

uint8_t buttons = 0;
int16_t axis[4] = {4};
uint8_t telemetry_packet[TELEMETRY_PACKET_SIZE] = {0}; 

/*------------------------------------------------------------------
 * process_{joystick, key} -- process command keys, mode change, or joystick
 * Jonathan Lévy
 * April 2018
 *------------------------------------------------------------------
 */

void process_joystick_axis(uint8_t *val)
{
	for (int i = 0; i < 4; i++)
	{
		int16_t stickvalue = (((int16_t) *(val + 2*i)) << 8) + ((int16_t) *(val + 2*i + 1));
		axis[i] = stickvalue;
	}
}

void process_joystick_button(uint8_t *val)
{
	buttons = *val;
}




void process_key(uint8_t *val)
{
	
	if (*val >= '0' && *val <= '8')
	{
		mode = *val;

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
			case 0:
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
	uint32_t tx_timer = 0;

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
				printf("%3d %3d %3d %3d | ",ae[0],ae[1],ae[2],ae[3]);
				
				printf("%6d %6d %6d | ", phi, theta, psi);
				printf("%6d %6d %6d | ", sp, sq, sr);
				printf("%4d | %4ld | %6ld \n", bat_volt, temperature, pressure);
				
				//printf("%d %d %d %d |", axis[0], axis[1], axis[2], axis[3] );

				//printf("%d |", buttons);
 
				printf("%d | ", mode);
				printf("\n");
			#endif

			//Filling rotor RPM data
			for (int j = 0; j < 4; j++)
			{
				telemetry_packet[ROTOR1 + 2*j] = MSBYTE( ae[j] );
				telemetry_packet[ROTOR1 + 2*j + 1] = LSBYTE( ae[j] );
			}	

			//Attitude
			telemetry_packet[PHI] = MSBYTE(phi);
			telemetry_packet[PHI + 1] = LSBYTE(phi);
			telemetry_packet[THETA] = MSBYTE(theta);
			telemetry_packet[THETA + 1] = LSBYTE(theta);
			telemetry_packet[PSI] = MSBYTE(psi);
			telemetry_packet[PSI + 1] = LSBYTE(psi);

			//Angular velocity
			telemetry_packet[SP] = MSBYTE(sp);
			telemetry_packet[SP + 1] = LSBYTE(sp);
			telemetry_packet[SQ] = MSBYTE(sq);
			telemetry_packet[SQ + 1] = LSBYTE(sq);
			telemetry_packet[SR] = MSBYTE(sr);
			telemetry_packet[SR + 1] = LSBYTE(sr);
			
			telemetry_packet[BAT_VOLT] = MSBYTE(bat_volt);
			telemetry_packet[BAT_VOLT + 1] = MSBYTE(bat_volt);

			telemetry_packet[TEMPERATURE] = MSBYTE_WORD(temperature);
			telemetry_packet[TEMPERATURE + 1] = BYTE2_WORD(temperature);
			telemetry_packet[TEMPERATURE + 2] = BYTE3_WORD(temperature);
			telemetry_packet[TEMPERATURE + 3] = LSBYTE_WORD(temperature);

			telemetry_packet[PRESSURE] = MSBYTE_WORD(pressure);
			telemetry_packet[PRESSURE + 1] = BYTE2_WORD(pressure);
			telemetry_packet[PRESSURE + 2] = BYTE3_WORD(pressure);
			telemetry_packet[PRESSURE + 3] = LSBYTE_WORD(pressure);
			
			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			run_filters_and_control();
		}

		if ((get_time_us() - tx_timer) > 100000)
		{
			tx_timer = get_time_us();
			send_telemetry_packet();
		}
	}	

	printf("\n\t Goodbye \n\n");
	nrf_delay_ms(100);

	NVIC_SystemReset();
}
