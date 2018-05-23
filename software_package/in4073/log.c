/*------------------------------------------------------------
 *log.c - Logging functionality to run on the board   
 *Logging data is put into the flash
 * Author - Tuan Anh Nguyen
 *
 * May 2018
 *------------------------------------------------------------
 */

#include "in4073.h"

#define MAX_ITEM 3400
#define LOG_TIME 0
#define LOG_SENSOR 4

//#define DEBUG

uint32_t logsize;
uint32_t current_address;


bool log_init() {	
	logsize = 0;
    current_address = 0;
    flash_chip_erase();
    return spi_flash_init();
}

bool log_write_item() {
    if (logsize > 3400){
        printf("Flash memory full");
        return false;
    }

    #ifdef DEBUG
    printf("Data to be logged: ");
    for (int i = 0; i<TELEMETRY_PACKET_SIZE;i++){
        printf("%X ", telemetry_packet[i]);
    }
    printf("\n");
    #endif

    bool write_status = flash_write_bytes(current_address, telemetry_packet, TELEMETRY_PACKET_SIZE);
    if (write_status){
        logsize++;
        current_address += TELEMETRY_PACKET_SIZE;

        #ifdef DEBUG
        printf("Log successful.\n");
        #endif
        //reset packet	
        for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
        {
            telemetry_packet[j] = 0;	
        }
        return true;
    } else {

        #ifdef DEBUG
        printf("Error writing to flash!");
        #endif

        //reset packet	
        for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
        {
            telemetry_packet[j] = 0;	
        }
        return false;
    }
}


bool log_sensor(){
    #ifdef DEBUG
    printf("Data to be logged: ");
    printf("||");
    printf("\n");
    #endif

    bool write_status = flash_write_bytes(current_address, telemetry_packet, TELEMETRY_PACKET_SIZE);
    if (write_status){
        logsize++;
        current_address += TELEMETRY_PACKET_SIZE;

        #ifdef DEBUG
        printf("Log successful.\n");
        #endif
        //reset packet	
        for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
        {
            telemetry_packet[j] = 0;	
        }
        return true;
    } else {

        #ifdef DEBUG
        printf("Error writing to flash!");
        #endif

        //reset packet	
        for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
        {
            telemetry_packet[j] = 0;	
        }
        return false;
    }
}

void log_read_last(){
    uint8_t buffer[TELEMETRY_PACKET_SIZE] = {0};
    uint32_t reading_address = current_address - TELEMETRY_PACKET_SIZE;
    printf("Reading log. Address %6ld.", reading_address);
    if(flash_read_bytes(reading_address, buffer,TELEMETRY_PACKET_SIZE)){
        for (int i = 0; i<TELEMETRY_PACKET_SIZE;i++){
            printf("%X ", buffer[i]);
        }
    }
}

void log_read_all(){
    uint8_t buffer[TELEMETRY_PACKET_SIZE] = {0};
    uint32_t reading_address = 0;
    printf(">>>>> Reading log. %6ld items. Size %6ld.",logsize,current_address);

    for (int i = 0; i<logsize; i++){
        if (flash_read_bytes(reading_address, buffer,TELEMETRY_PACKET_SIZE)){
            for(int count = 0; count <TELEMETRY_PACKET_SIZE;count++){
                printf("%X ", buffer[count]);
                buffer[count] = 0;
            }
            printf("\n");
            reading_address += TELEMETRY_PACKET_SIZE;
        }
    }

}