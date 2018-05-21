#define CONTROL_PACKET_SIZE 13
#define TELEMETRY_PACKET_SIZE 36

#define START 0
#define KEY 1
#define MODE 2
#define JOYBUTTON 3
// #define AXISTHROTTLE 4
// #define AXISROLL 6
// #define AXISPITCH 8
// #define AXISYAW 10
#define AXISROLL 4
#define AXISPITCH 6
#define AXISYAW 8
#define AXISTHROTTLE 10

#define CRC 12

#define _STARTBYTE 0xFF

#define DELAY_PACKET_NS 20000000

#define ROTOR1 1
#define ROTOR2 3
#define ROTOR3 5
#define ROTOR4 7

#define PHI 9
#define THETA 11
#define PSI 13
#define SP 15
#define SQ 17
#define SR 19

#define BAT_VOLT 21
#define TEMPERATURE 23
#define PRESSURE 27
#define TIMESTAMP 31
#define CRC_TELEMETRY 35

#define MSBYTE(x) ((uint8_t) ((x & 0xff00) >> 8))

#define LSBYTE(x) ((uint8_t) ((x & 0x00ff)))

#define MSBYTE_WORD(x) ((uint8_t) ((x & 0xff000000) >> 24))
#define BYTE2_WORD(x) ((uint8_t) ((x & 0x00ff0000) >> 16))
#define BYTE3_WORD(x) ((uint8_t) ((x & 0x0000ff00) >> 8))
#define LSBYTE_WORD(x) ((uint8_t) ((x & 0x000000ff)))
