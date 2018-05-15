#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <signal.h>
#include "packet_constants.h"

uint8_t control_packet[CONTROL_PACKET_SIZE]; //packet to be sent to drone over serial
void process_telemetry(uint8_t c);
void send_packet();
int rs232_putchar(char c);
