


#define MOTORS_OFF (motor[3] == 0 && motor[2] == 0 && motor[1] == 0 && motor[0] == 0)
#define MODE_0_SAFE 0
#define MODE_1_PANIC 1
#define MODE_2_MANUAL 2
#define MODE_3_CALIB 3
#define MODE_4_YAWCTRL 4
#define MODE_5_FULLCTRL 5
#define MODE_6_RAW 6


void mode_0_safe_RUN(void);
void mode_1_panic_RUN(void);
void mode_2_manual_RUN(void);

void mode_0_safe_QUIT(void);
void mode_1_panic_QUIT(void);
void mode_2_manual_QUIT(void);

void mode_0_safe_INIT(void);
void mode_1_panic_INIT(void);
void mode_2_manual_INIT(void);

char mode_0_safe_CANLEAVE(uint8_t target);
char mode_0_safe_CANENTER(uint8_t source);
char mode_1_panic_CANLEAVE(uint8_t target);
char mode_1_panic_CANENTER(uint8_t source);
char mode_2_manual_CANLEAVE(uint8_t target);
char mode_2_manual_CANENTER(uint8_t source);