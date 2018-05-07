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

static uint8_t packet[SIZEOFPACKET] = {0};
static uint8_t index = 0;
static uint8_t crc = 0;
static uint16_t last_OK_packet = 0;


void send_ack(){
    last_OK_packet = (MSBYTE(packet[PACKETID])) + LSBYTE(packet[PACKETID]) ; // not using TOSHORT because I'm not sure using macros of macros would work. Preprocessing is a bit hairy.
    
    uint8_t ack_CRC = 0xff ^ MSBYTE(packet[PACKETID]) ^ LSBYTE(packet[PACKETID]);
    printf("%c%c%c%c", 0xff, MSBYTE(last_OK_packet), LSBYTE(last_OK_packet), ack_CRC);
}


void process_packet(uint8_t c) {
    // Packet beginning detection.


    if (index == 0 && c != 0xff)
    {
        return ;
    }

    if (index < SIZEOFPACKET)
    {
        packet[index] = c;
        crc = crc ^ c;
        index = (index+1); // in any case, don't go over SIZEOFPACKET.
    }

    if (index == SIZEOFPACKET) // we got a full packet, and it passes the CRC test! 
    {
        #ifdef DEBUG
        printf("packet got~~~ : ");
            for (int j = 0; j < SIZEOFPACKET; j++)
            {
                printf("%X ", packet[j]);
            }
        printf(" ~ crc = %X",crc); 
        #endif

        if (crc == 0) {
            #ifdef DEBUG

            // passing. Fine. Don't care.          
            #endif
            #ifndef DEBUG 
            // passing by pointer is simpler and faster.
            process_key(packet + KEY);

            process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

            process_joystick_button(packet + JOYBUTTON);
            #endif

        }  else if (crc != 0) {
        #ifdef DEBUG
            // not passing. Pin-pon-error-blink-red
            printf(" - crc fail.");
            nrf_gpio_pin_toggle(RED);

        #endif
    }
            printf("\n");
            crc = 0;
            index = 0;
            for (int i = 0; i < SIZEOFPACKET; i++)
                packet[i] = 0;
    }
    

    
}