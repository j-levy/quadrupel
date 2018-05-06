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

// #define DEBUG

static uint8_t packet[SIZEOFPACKET] = {0};
static uint8_t index = 0;
static uint8_t crc = 0;
static uint16_t last_OK_packet = 0;


// the index indicates the first FREE slot in the packet array.
// We can fill the array using it.
uint8_t CRC_pass() {
    
    uint8_t isValid = ~crc;

    // empty UART if there's more stuff inside !
    while (rx_queue.count) 
			dequeue(&rx_queue);
    
    return isValid;
}

void send_ack(){
    last_OK_packet = (MSBYTE(packet[PACKETID])) + LSBYTE(packet[PACKETID]) ; // not using TOSHORT because I'm not sure using macros of macros would work. Preprocessing is a bit hairy.
    
    uint8_t ack_CRC = 0xff ^ MSBYTE(packet[PACKETID]) ^ LSBYTE(packet[PACKETID]);
    printf("%c%c%c%c", 0xff, MSBYTE(last_OK_packet), LSBYTE(last_OK_packet), ack_CRC);
}


void process_packet(uint8_t c) {
    
    // printf("\n====================================\n\t\tRead : %x, index : %d\n====================================\n", c, index);

    // Packet beginning detection.
    if (index == 0 && c != 0xff)
        return ;
    
    if (index < SIZEOFPACKET)
    {
        packet[index] = c;
        crc = crc ^ c;
        index = (index+1); // in any case, don't go over SIZEOFPACKET.
    }

    if (index == SIZEOFPACKET) // we got a full packet, and it passes the CRC test! 
    {
        crc = 0;
        index = 0;

        #ifdef DEBUG
            // display
            printf("packet got  : ");
            for (int j = 0; j < SIZEOFPACKET; j++)
            {
                printf("%X ", packet[j]);
            }
            printf("\n");
        #endif
        
        // passing by pointer is simpler and faster.
        process_key(packet + KEY);

        process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

        process_joystick_button(packet + JOYBUTTON);
        
        /// send_ack();

        // clean packet
        for (int i = 0; i < SIZEOFPACKET; i++)
            packet[i] = 0;
    }  

    
}