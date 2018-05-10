/*------------------------------------------------------------------
 *  packet.c -- a packet parser, byte by byte.
 *
 *  J.Lévy
 *  Embedded Software Lab - student
 *
 *  April 2018
 *------------------------------------------------------------------
 */

#include "in4073.h"

#define DEBUG
//#define DEBUGACK

static uint8_t packet[CONTROL_PACKET_SIZE] = {0};
static uint8_t index = 0;
static uint8_t crc = 0;
//static uint8_t count = 0;
static uint8_t packet_length = CONTROL_PACKET_SIZE; //indicates packet length to parse including the offset if any
                                                    //initialized with 13 (no offset)
                                                    //Offset comes from the previous cycle of partially parsed packet
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



/*
 * process incoming packets
 * Revised: 9th May 2018 - Tuan Anh Nguyen
*/

void process_packet(uint8_t c) {
    // Packet beginning detection.

    uint8_t l = 0;
    //printf("Character read: %X \n", c);
    if (index == 0 && c != _STARTBYTE)
    {
        return ;
    }

    if (index < packet_length)
    {
        packet[index] = c;
        // if((index == 7) && (count == 150)) {
        //     crc = 9;
        //  }
        // else
        crc = crc ^ c;

        index = (index+1); // in any case, don't go over SIZEOFPACKET.    
    }

    if (index == packet_length) // we got a full packet, and it passes the CRC test! 
    {
        #ifdef DEBUG
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
            l = CONTROL_PACKET_SIZE;
            #ifndef DEBUG 

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

            crc = 0;  //recompute crc
            l = 1;  // starting from the next byte after previous start byte
            while(packet[l] != _STARTBYTE)  //find the index of next startbyte starting 
            {                               //from the byte after the previous start byte
                l++;
            }      //'l' could be < or = 13 , if l=13, 
                   //we continue as usual with the new arriving byte                        
            
            if(l < packet_length)
            {
                //Compute crc from this new start byte till the last byte in our current array       
                for(int i = l; i < packet_length; i++)
                {
                    crc = crc ^ packet[i];
                }
                packet_length = l + CONTROL_PACKET_SIZE;  //reset packet length for full packet traversal
            }
            
        } 
    
        index = CONTROL_PACKET_SIZE - l;
        for(int i = 0; i < l; i++)  //discard/reset unwanted part of packet
        {
            packet[i] = 0;
        }

    }
        //count++;
}