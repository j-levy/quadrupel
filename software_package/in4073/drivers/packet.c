/*------------------------------------------------------------------
 *  packet.c : process_packet() method parses the recived control packet byte by byte,
 *             does CRC computation according to the logic explained below and validates 
 *             correct reception of packets.
 *  
 *  Authors and contribution:
 *  Niket Agrawal (70%)
 *  Tuan Anh Nguyen (30%)
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

//#define DEBUG
//#define DEBUGACK
//#define DEBUGCRC      //Enable CRC decoding test here
#define ENABLE_STORE_DATA

static uint8_t packet[CONTROL_PACKET_SIZE] = {0};
static uint8_t index = 0;
static uint8_t crc = 0;
#ifdef DEBUGCRC
static int count = 0;
#endif
static uint8_t l = 0;  //variable used to determine new startbyte

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
        #ifdef DEBUG
            printf("control packet received~~~ : ");
            for (int j = 0; j < CONTROL_PACKET_SIZE; j++)
            {
                printf("%X ", packet[j]);
            }
            printf(" ~ crc = %X \n",crc); 
        #endif

        if (crc == 0) {
            
            #ifdef ENABLE_STORE_DATA 

            store_key(packet + KEY);
            store_mode(packet + MODE);
            store_joystick_axis(packet + AXISROLL); // roll is the first value.
            store_joystick_button(packet + JOYBUTTON);

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
                l = 0; //reset l to index 0 again
                //CRC computation done for the 1st half of the packet, new bytes arriving
                //to prpcess_packet method will form the 2nd half of this packet and CRC 
                //computation will continue from (index < size) check in line 67
                
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

