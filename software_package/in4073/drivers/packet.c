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

static uint8_t packet[SIZEOFPACKET] = {0};
static uint8_t index = 0;
static uint8_t crc = 0;
static uint16_t last_OK_packet = 0;


// the index indicates the first FREE slot in the packet array.
// We can fill the array using it.
uint8_t CRC_pass() {
    index = 0;
    uint8_t isValid = ~crc;
    crc = 0;
    // empty UART if there's more stuff inside !
    while (rx_queue.count) 
			dequeue(&rx_queue);
    
    return isValid;
}

void send_ack(){
    last_OK_packet = (MSBYTE(packet[PACKETID]) << 8) + LSBYTE(packet[PACKETID]) ; // not using TOSHORT because I'm not sure using macros of macros would work. Preprocessing is a bit hairy.
    
    uint8_t ack_CRC = 0xff ^ MSBYTE(packet[PACKETID]) ^ LSBYTE(packet[PACKETID]);
    printf("%c%c%c%c", 0xff, MSBYTE(last_OK_packet), LSBYTE(last_OK_packet), ack_CRC);
}


void process_packet(uint8_t c) {
    
    printf("\n====================================\n\t\tRead : %x, index : %d\n====================================\n", c, index);

    // Packet beginning detection.
    if (index == 0 && c != 0xff)
        return ;
    

    packet[index] = c;
    crc = crc ^ c;


    if (index == SIZEOFPACKET-1 && CRC_pass()) // we got a full packet, and it passes the CRC test! 
    {

        // passing by pointer is simpler and faster.
        process_key(packet + KEY);

        process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

        process_joystick_button(packet + JOYBUTTON);

        // sending the acknowledgement is part of managing packet, so it's an local function.
        // It can use the static variables, hence no need for argument.
        send_ack();
    }  

    index = (index >= SIZEOFPACKET-1) ? 0 : index + 1; // in any case, don't go over SIZEOFPACKET.
}