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

//#define DEBUG
//#define DEBUGACK

static uint8_t packet[SIZEOFPACKET] = {0};
static uint8_t index = 0;
static uint8_t crc = 0;

void send_ack(){
    
    /* WARNING : the pc_terminal reads the characters as int. SIGNED INT.
        Plus, the code -1 (which is, BAD LUCK, corresponding to 0xFF) means no character received.
        This is why I changed the start bit to 0xF0.
    */
    uint8_t ack_CRC = 0xf0 ^ packet[PACKETID] ^ packet[PACKETID+1];
    #ifdef DEBUGACK
    printf("%x %x %x %x", 0xff, packet[PACKETID], packet[PACKETID+1], ack_CRC);
    uart_put(0xff);
    uart_put(packet[PACKETID]);
    uart_put(packet[PACKETID+1]);
    uart_put(ack_CRC);
    #endif
    #ifndef DEBUGACK
    uart_put(0xF0);
    uart_put(packet[PACKETID]);
    uart_put(packet[PACKETID+1]);
    uart_put(ack_CRC);
    #endif
    
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
            send_ack();

            // passing. Fine. Don't care.          
            #endif
            #ifndef DEBUG 
            send_ack();
            // passing by pointer is simpler and faster.
            /*
            process_key(packet + KEY);

            process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

            process_joystick_button(packet + JOYBUTTON);
            */
            #endif

        }  else if (crc != 0) {
        #ifdef DEBUG

            // not passing. Pin-pon-error-blink-red
                printf(" - crc fail.");
            nrf_gpio_pin_toggle(RED);

        #endif
    }
            // printf("\n");

            crc = 0;
            index = 0;
            for (int i = 0; i < SIZEOFPACKET; i++)
                packet[i] = 0;
    }
    

    
}