/*------------------------------------------------------------------
 *  packet.c -- a packet parser, byte by byte.
 *
 *  Author: Niket Agrawal, Jonathan Levy
 * 
 * CRC is computed byte by byte and CRC check is done at the end of each packet (13 bytes) as before. 
 * The idea is to NOT discard the whole packet upon CRC decoding failure, as we may have picked a wrong start byte 
 * and assumed that a new packet has started. If we throw the packet, we may loose the correct start byte which could 
 * be somewhere within the packet.
 * if a CRC check failure is detected, the next start byte is found starting from the previous start byte.
 * If a start byte (0xFF) is detected within the current packet, this means we re-use a chunk of this packet. 
 * New packet starts from this new start byte. All the bytes before this are discarded
 * CRC is computed for the left over bytes first (from new start byte to end of packet). 
 * Rest half of the packet is formed by new bytes arriving to process packet method and CRC is computed in usual manner.
 *
 *  May 2018
 *------------------------------------------------------------------
 */

#include "in4073.h"

//#define DEBUGCONTROLRX
#define DEBUGTELEMETRYTX
//#define DEBUGCRC      //Enable CRC decoding test here
//#define DEBUGPROCESSINPUT

static uint8_t packet[CONTROL_PACKET_SIZE] = {0};
extern uint8_t telemetry_packet[TELEMETRY_PACKET_SIZE];
static uint8_t index = 0;
static uint8_t crc = 0;
#ifdef DEBUGCRC
static int count = 0;
#endif
static uint8_t l = 0;  //variable used to determine new startbyte

//static uint8_t last_OK[2] = {0,0};

// void send_ack(){
    
//     /* WARNING : the pc_terminal reads the characters as int. SIGNED INT.
//         Plus, the code -1 (which is, BAD LUCK, corresponding to 0xFF) means no character received.
//         This is why I changed the start bit to 0xF0.
//     */

//     uint8_t ack_CRC = _STARTBYTE ^ last_OK[0]  ^ last_OK[1];
//     #ifdef DEBUGACK
//     printf("%x %x %x %x", _STARTBYTE, packet[PACKETID], packet[PACKETID+1], ack_CRC);
//     #endif
//     //#ifndef DEBUGACK
//     // printf("%c%c%c%c", _STARTBYTE, a, b, ack_CRC);
    
//     uart_put4(_STARTBYTE, last_OK[0], last_OK[1], ack_CRC );

    
    //#endif
//}


void process_packet(uint8_t c) {
    // Packet beginning detection.

    
    //printf("Character read: %X \n", c);
    if (index == 0 && c != _STARTBYTE)
    {
        return ;
    }

    if (index < CONTROL_PACKET_SIZE)
    {
        packet[index] = c;
        #ifdef DEBUGCRC
        //Corrupt CRC for random byte to simulate error scenario
        if((count == 750) || (count == 150) || (count == 1300)) {
            crc = 9;
         }
        else
        #endif
        crc = crc ^ c;

        index = (index+1); // in any case, don't go over SIZEOFPACKET.    
    }

    if (index == CONTROL_PACKET_SIZE) //reached the end of a packet 
    {
        #ifdef DEBUGCONTROLRX
            printf("packet received~~~ : ");
            for (int j = 0; j < CONTROL_PACKET_SIZE; j++)
            {
                printf("%X ", packet[j]);
            }
            printf(" ~ crc = %X \n",crc); 
        #endif

        if (crc == 0) {
            // last_OK[0] = packet[PACKETID];
            // last_OK[1] = packet[PACKETID+1];
            // #ifdef DEBUG

            // send_ack(packet[PACKETID], packet[PACKETID+1]);

            // #endif
            //l = CONTROL_PACKET_SIZE;
            #ifdef DEBUGPROCESSINPUT 

            // send_ack(packet[PACKETID], packet[PACKETID+1]);

            process_key(packet + KEY);

            process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

            process_joystick_button(packet + JOYBUTTON);

            #endif

        }  
        else if (crc != 0)
        {
            #ifdef DEBUG
            // not passing. Pin-pon-error-blink-red
            printf(" - crc fail.");
            nrf_gpio_pin_toggle(RED);
            #endif

            crc = 0;  //reset crc as it will be recomputed
            l = l + 1;  // begin from the next byte after the previous start byte
            while(packet[l] != _STARTBYTE)  //find the index of next startbyte starting 
            {                               //from the byte after the previous start byte
                //printf("iteration\n");
                l++;
            }      

            #ifdef DEBUGCRC                        
            printf("Next start byte found at %d\n", l);
            #endif

            //if next start byte was found within the current packet
            if(l < CONTROL_PACKET_SIZE)
            {
                #ifdef DEBUGCRC
                printf("l is less than packet length\n");
                #endif
                //Compute crc from this new start byte till the last byte in our current array
                index = 0;    //index is zero for new start byte, new packet starts from here   
                for(int i = l; i < CONTROL_PACKET_SIZE; i++)
                {
                    packet[i-l] = packet[i];  //overwrite previous bytes
                    crc = crc ^ packet[i];
                    index = index + 1;
                }
                //CRC computation done for the 1st half of the packet, new bytes arriving
                //to prpcess_packet method will form the 2nd half of this packet and CRC 
                //computation will continue from (index < size) check in line 59
                
            }

            else //no other start byte found in the current packet
                 //reset index to zero and clear packet 
            {
                index = 0;
                for(int i = 0; i < CONTROL_PACKET_SIZE; i++)
                {
                    packet[i] = 0;
                }
            }
            
        } 
        
    }

    // reset index to zero and clear packet if index reached the end of packet,
    // this is required for case when there is no CRC error and a full packet is
    // parsed and decoded correctly
    if(index == CONTROL_PACKET_SIZE)
    {
        index = 0;
        for(int i = 0; i < l; i++)  
        {
            packet[i] = 0;
        }
    }
    #ifdef DEBUGCRC
    count++; 
    #endif
}



/*------------------------------------------------------------
 * Method to process the received ACK packet from Drone 
 *
 * Author - Niket Agrawal
 *
 * Sends the telemetry data to the PC at 10Hz
 * 
 *------------------------------------------------------------
 */
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
    #ifdef DEBUGTELEMETRYTX
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