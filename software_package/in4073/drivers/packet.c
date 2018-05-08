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
static uint8_t last_OK[2] = {0,0};

void send_ack(){
    
    /* WARNING : the pc_terminal reads the characters as int. SIGNED INT.
        Plus, the code -1 (which is, BAD LUCK, corresponding to 0xFF) means no character received.
        This is why I changed the start bit to 0xF0.
    */

    uint8_t ack_CRC = _STARTBYTE ^ last_OK[0]  ^ last_OK[1];
    #ifdef DEBUGACK
    printf("%x %x %x %x", _STARTBYTE, packet[PACKETID], packet[PACKETID+1], ack_CRC);
    #endif
    //#ifndef DEBUGACK
    // printf("%c%c%c%c", _STARTBYTE, a, b, ack_CRC);
    
    uart_put4(_STARTBYTE, last_OK[0], last_OK[1], ack_CRC );

    
    //#endif
}


void process_packet(uint8_t c) {
    // Packet beginning detection.


    if (index == 0 && c != _STARTBYTE)
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
            last_OK[0] = packet[PACKETID];
            last_OK[1] = packet[PACKETID+1];
            #ifdef DEBUG

            send_ack(packet[PACKETID], packet[PACKETID+1]);

            #endif
            #ifndef DEBUG 

            send_ack(packet[PACKETID], packet[PACKETID+1]);

            process_key(packet + KEY);

            process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

            process_joystick_button(packet + JOYBUTTON);

            #endif

        }  else if (crc != 0) {
            send_ack();
        #ifdef DEBUG
            
            // not passing. Pin-pon-error-blink-red
            // printf(" - crc fail.");
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