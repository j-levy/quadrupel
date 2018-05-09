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

    //printf("Character read: %X \n", c);
    if (index == 0 && c != _STARTBYTE)
    {
        return ;
    }

    if (index < CONTROL_PACKET_SIZE)
    {
        packet[index] = c;
        crc = crc ^ c;
        index = (index+1); // in any case, don't go over SIZEOFPACKET.    
    }

    if (index == CONTROL_PACKET_SIZE) // we got a full packet, and it passes the CRC test! 
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
            #ifndef DEBUG 

            // send_ack(packet[PACKETID], packet[PACKETID+1]);

            process_key(packet + KEY);

            process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

            process_joystick_button(packet + JOYBUTTON);

            #endif

        }  else if (crc != 0) {                             //If a decode error is detected
            crc = 0 ^ packet[0];                            //discard the start byte and begin decoding from the following byte
            for (int i = 0; i <CONTROL_PACKET_SIZE-1; i++){ //could be better to start decoding from the next start byte instead
                packet[i]= packet[i+1];
                crc = crc ^ packet[i+1];
                printf("%X",packet[i]);
            }
            index--;
        #ifdef DEBUG
            
            // not passing. Pin-pon-error-blink-red
            printf(" - crc fail.");
            nrf_gpio_pin_toggle(RED);

        #endif
    }
            // printf("\n");

            crc = 0;
            index = 0;
            for (int i = 0; i < CONTROL_PACKET_SIZE; i++)
                packet[i] = 0;
    }
    

    
}