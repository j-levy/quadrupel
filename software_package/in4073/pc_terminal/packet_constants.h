#define SIZEOFPACKET 14

#define START 0

#define PACKETID 1

#define KEY 3

#define JOYBUTTON 4

#define AXISTHROTTLE 5

#define AXISROLL 7

#define AXISPITCH 9

#define AXISYAW 11

#define CRC 13

#define DELAYSENDPACKET 20

#define MSBYTE(x) ((uint8_t) ((x & 0xff00) >> 8))

#define LSBYTE(x) ((uint8_t) ((x & 0x00ff)))

