#define CONTROL_PACKET_SIZE 13
#define TELEMETRY_PACKET_SIZE 28

#define START 0
#define KEY 1
#define MODE 2
#define JOYBUTTON 3
#define AXISTHROTTLE 4
#define AXISROLL 6
#define AXISPITCH 8
#define AXISYAW 10
#define CRC 12

#define _STARTBYTE 0xFF

#define DELAY_PACKET_NS 50000000

#define MSBYTE(x) ((uint8_t) ((x & 0xff00) >> 8))

#define LSBYTE(x) ((uint8_t) ((x & 0x00ff)))
