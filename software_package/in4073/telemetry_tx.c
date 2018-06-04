/*------------------------------------------------------------
 *telemtery_tx.c - Telemetry data transmission to PC   
 *
 * Author - Niket Agrawal
 *
 * May 2018
 *------------------------------------------------------------
 */

#include "in4073.h"

//#define DEBUG

void send_telemetry_packet()
{
    telemetry_packet[START] = _STARTBYTE;
    uint32_t current_time = get_time_us();
	telemetry_packet[TIMESTAMP] = MSBYTE_WORD(current_time);
	telemetry_packet[TIMESTAMP + 1] = BYTE2_WORD(current_time);
	telemetry_packet[TIMESTAMP + 2] = BYTE3_WORD(current_time);
	telemetry_packet[TIMESTAMP + 3] = LSBYTE_WORD(current_time);

	uint8_t crc = 0;

	for(int i = 0; i < TELEMETRY_PACKET_SIZE; i++)
	{
		crc = crc ^ telemetry_packet[i];
	}
	telemetry_packet[CRC_TELEMETRY] = crc;   
	int j = 0;
    while(j < TELEMETRY_PACKET_SIZE)
	{
        uart_put(telemetry_packet[j]);
		j++;
	}
    #ifdef DEBUG
		// display the packet that is sent
		//fprintf(stderr, "telemetry packet sent : ");
        printf("telemetry packet sent : ");
		for (int k = 0; k < TELEMETRY_PACKET_SIZE; k++)
		{
			//fprintf(stderr, "%X ", telemetry_packet[k]);
            printf("%X ", telemetry_packet[k]);
		}
		//fprintf(stderr, "\n");
        printf("\n");
	#endif
	//reset packet	
	for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
	{
		telemetry_packet[j] = 0;	
	}

}