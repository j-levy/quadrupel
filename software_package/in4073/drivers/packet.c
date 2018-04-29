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
static uint8_t packet[6] = {0};
static uint8_t index = 0;
static uint8_t CRC = 0;
// the index indicates the first FREE slot in the packet array.
// We can fill the array using it.
uint8_t CRC_pass() {
    index = 0;
    uint8_t isValid = ~CRC;
    CRC = 0;
    
    return isValid;
    //return 1; // Test just to see if the parsing is correct. no CRC validation.
}


void process_packet(uint8_t c) {
    
    if (index == 0 && c != 0xff)
        return ;

    packet[index] = c;
    CRC = CRC ^ c;

    if (index == 3 && packet[1] == 0) // packet type "Mode" fully received.
    {
        if (CRC_pass())
            process_mode(packet[2]);
    }

    if (index == 3 && packet[1] == 2) // packet type "key" fully received.
    {
        if (CRC_pass())
            process_key(packet[2]);
    }

    if (index == 5 && packet[1] == 1) // packet type "joystick" fully received
    {
        int16_t value = (packet[3] + (((int16_t) packet[4])<<8));
        if (CRC_pass())
            process_joystick(packet[2], value);
    }
    printf("\n====================================\n\t\tRead : %x, index : %d\n====================================\n", c, index);

    index = (index >= 5) ? 0 : index + 1; // in any case, don't go over 5.
}