/*------------------------------------------------------------------
 *  in4073.h -- defines, globals, function prototypes
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#ifndef IN4073_H__
#define IN4073_H__

#include <inttypes.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml.h"
#include "app_util_platform.h"
#include <math.h>


// offsets to access the bytes in the packet easily.
#include "./pc_terminal/packet_constants.h"

#define RED		    22
#define YELLOW		24
#define GREEN		28
#define BLUE		30
#define INT_PIN		5

bool demo_done;

// Control
int16_t motor[4],ae[4], offset[4];

uint8_t buttons;
uint8_t keyboard_key;
int16_t axis[4];

void run_filters_and_control();

// Timers
#define TIMER_PERIOD	20 //20ms = 50 Hz. 50ms=20Hz (MAX 23bit, 4.6h)
void timers_init(void);
uint32_t get_time_us(void);
bool check_timer_flag(void);
void clear_timer_flag(void);

// GPIO
void gpio_init(void);

// Queue
#define QUEUE_SIZE 256
typedef struct {
	uint8_t Data[QUEUE_SIZE];
	uint16_t first,last;
  	uint16_t count; 
} queue;
void init_queue(queue *q);
void enqueue(queue *q, char x);
char dequeue(queue *q);

// UART
#define RX_PIN_NUMBER  16
#define TX_PIN_NUMBER  14
queue rx_queue;
queue tx_queue;
uint32_t last_correct_checksum_time;
void uart_init(void);
void uart_put(uint8_t);
void uart_put4(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3 );


// TWI
#define TWI_SCL	4
#define TWI_SDA	2
void twi_init(void);
bool i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const *data);
bool i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data);

// MPU wrapper
int16_t phi, theta, psi;
int16_t sp, sq, sr;
int16_t sax, say, saz;
uint8_t sensor_fifo_count;
void imu_init(bool dmp, uint16_t interrupt_frequency); // if dmp is true, the interrupt frequency is 100Hz - otherwise 32Hz-8kHz
void get_dmp_data(void);
void get_raw_sensor_data(void);
bool check_sensor_int_flag(void);
void clear_sensor_int_flag(void);

// Barometer
int32_t pressure;
int32_t temperature;
void read_baro(void);
void baro_init(void);

// ADC
uint16_t bat_volt;
void adc_init(void);
void adc_request_sample(void);

// Flash
bool spi_flash_init(void);
bool flash_chip_erase(void);
bool flash_write_byte(uint32_t address, uint8_t data);
bool flash_write_bytes(uint32_t address, uint8_t *data, uint32_t count);
bool flash_read_byte(uint32_t address, uint8_t *buffer);
bool flash_read_bytes(uint32_t address, uint8_t *buffer, uint32_t count);

// BLE
queue ble_rx_queue;
queue ble_tx_queue;
volatile bool radio_active;
void ble_init(void);
void ble_send(void);


// Mode
uint8_t mode;

// Packet Parser [Jonathan LEVY]
void process_packet(uint8_t c);

void store_key(uint8_t *val);
void store_joystick_axis(uint8_t *val);
void store_joystick_button(uint8_t *val);
void store_mode(uint8_t *val);

void send_ack();


// mode functions
void init_modes();
void switch_mode( uint8_t newmode );

typedef void (*void_ptr_t)(void);
typedef char (*char_ptr_t)(uint8_t);
void_ptr_t mode_RUN[7];
void_ptr_t mode_INIT[7];
void_ptr_t mode_QUIT[7];
char_ptr_t mode_CANLEAVE[7];
char_ptr_t mode_CANENTER[7];


#define ROLL 0
#define PITCH 1
#define YAW 2
#define LIFT 3

void update_motors(void);

#endif // IN4073_H__
