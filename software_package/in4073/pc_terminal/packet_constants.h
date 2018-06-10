#define CONTROL_PACKET_SIZE 13
#define TELEMETRY_PACKET_SIZE 23

#define START 0
#define KEY 1
#define MODE 2
#define JOYBUTTON 3
#define AXISROLL 4
#define AXISPITCH 6
#define AXISYAW 8
#define AXISTHROTTLE 10

#define CRC 12

#define _STARTBYTE 0xFF

#define DELAY_PACKET_NS 20000000
#define TELEMETRY_TIMEOUT_NS 750000000

#define ROTOR1 1
#define ROTOR2 3
#define ROTOR3 5
#define ROTOR4 7
#define MODE_DRONE 9
#define BAT_VOLT 10
#define TIMESTAMP 12
#define P_YAW 16
#define P1 18
#define P2 20
#define CRC_TELEMETRY 22

#define MSBYTE(x) ((uint8_t) ((x & 0xff00) >> 8))

#define LSBYTE(x) ((uint8_t) ((x & 0x00ff)))

#define MSBYTE_WORD(x) ((uint8_t) ((x & 0xff000000) >> 24))
#define BYTE2_WORD(x) ((uint8_t) ((x & 0x00ff0000) >> 16))
#define BYTE3_WORD(x) ((uint8_t) ((x & 0x0000ff00) >> 8))
#define LSBYTE_WORD(x) ((uint8_t) ((x & 0x000000ff)))
