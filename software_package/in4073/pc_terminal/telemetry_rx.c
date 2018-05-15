/*------------------------------------------------------------
 * telemtery_rx.c -> Parses and Process the telemetry data
 * received from drone  
 *
 * Author - Niket Agrawal
 *
 * May 2018
 *------------------------------------------------------------
 */
#include "pc_terminal.h"
#include "packet_constants.h"

#define DEBUGCRC
#define DEBUG
//#define DEBUGCRC

uint8_t packet_rx[TELEMETRY_PACKET_SIZE] = {0};  //char array of telemetry packet size
                                                 //to store a packet locally during processing
static uint8_t index_parser = 0;
static uint8_t crc = 0;
#ifdef DEBUGCRC
static int count = 0;
#endif
static uint8_t l = 0;  //variable used to determine new startbyte

void process_telemetry(uint8_t c)
{
	if (index_parser == 0 && c != _STARTBYTE)
    {
        return ;
    }

    if (index_parser < TELEMETRY_PACKET_SIZE)
    {
        packet_rx[index_parser] = c;
        #ifdef DEBUGCRC
        //Corrupt CRC for random byte to simulate error scenario
        if((count == 1000) || (count == 1500) || (count == 2200)) {
            crc = 9;
         }
        else
        #endif
        crc = crc ^ c;

        index_parser = (index_parser + 1); // in any case, don't go over SIZEOFPACKET.    
    }

    if (index_parser == TELEMETRY_PACKET_SIZE) //reached the end of a packet 
    {
        #ifdef DEBUG
            printf("telemetry packet received~~~ : ");
            for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
            {
                printf("%X ", packet_rx[j]);
            }
            printf(" ~ crc = %X \n",crc); 
        #endif

        if (crc == 0) 
		{
            //do something, which i can't remember right now
        }  
        else if (crc != 0)
        {
            #ifdef DEBUG
            // not passing. Pin-pon-error-blink-red
            printf(" - crc fail.");
            #endif

            crc = 0;  //reset crc as it will be recomputed
            l = l + 1;  // begin from the next byte after the previous start byte
            while(packet_rx[l] != _STARTBYTE)  //find the index of next startbyte starting 
            {                               //from the byte after the previous start byte
                //printf("iteration\n");
                l++;
            }      

            #ifdef DEBUGCRC                        
            printf("Next start byte found at %d\n", l);
            #endif

            if(l < TELEMETRY_PACKET_SIZE)
            {
                #ifdef DEBUGCRC
                printf("l is less than packet length\n");
                #endif
                //Compute crc from this new start byte till the last byte in our current array
                index_parser = 0;    //index is zero for new start byte, new packet starts from here   
                for(int i = l; i < TELEMETRY_PACKET_SIZE; i++)
                {
                    packet_rx[i-l] = packet_rx[i];  //overwrite previous bytes
                    crc = crc ^ packet_rx[i];
                    index_parser = index_parser + 1;
                }
                l = 0; //reset l to index 0 again
                //CRC computation done for the 1st half of the packet, new bytes arriving
                //to prpcess_packet method will form the 2nd half of this packet and CRC 
                //computation will continue from (index < size) check in line 33
            }

            else //no other start byte found in the current packet
                 //reset index to zero and clear packet 
            {
                index_parser = 0;
                for(int i = 0; i < TELEMETRY_PACKET_SIZE; i++)
                {
                    packet_rx[i] = 0;
                }
            }
            
        } 
        
    }

    // reset index to zero and clear packet if index reached the end of packet
    if(index_parser == TELEMETRY_PACKET_SIZE)
    {
        index_parser = 0;
        for(int i = 0; i < l; i++)  
        {
            packet_rx[i] = 0;
        }
    }
    #ifdef DEBUGCRC
    count++; 
    #endif
}