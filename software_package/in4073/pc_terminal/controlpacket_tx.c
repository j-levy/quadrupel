/*------------------------------------------------------------
 * controlpacket_tx.c -> Transmits control packet (input recorded 
 * from keyboard, joystick) to drone over serial 
 *
 * Author - Niket Agrawal
 * 
 * May 2018
 *------------------------------------------------------------
 */

#include "pc_terminal.h"
#include "packet_constants.h"

//#define DEBUG

void send_packet()
{
	control_packet[START] = _STARTBYTE;

	uint8_t crc = 0;

	for(int i = 0; i < CONTROL_PACKET_SIZE; i++)
	{
		crc = crc ^ control_packet[i];
	}
	control_packet[CRC] = crc;   
	int i = 0;

	while((rs232_putchar(control_packet[i]) == 1) && (i < CONTROL_PACKET_SIZE))
	{
		i++;
	}

	#ifdef DEBUG
		// display the packet that is sent
		fprintf(stderr, "control packet sent : ");
		for (int j = 0; j < CONTROL_PACKET_SIZE; j++)
		{
			fprintf(stderr, "%X ", control_packet[j]);
		}
		fprintf(stderr, "\n");
	#endif

	//reset packet	
	for (int j = 0; j < CONTROL_PACKET_SIZE; j++)
	{
		if(j != MODE)  //Retain the previous mode info
			control_packet[j] = 0;	
	}
	
}
