/*------------------------------------------------------------
 *log.c - Logging functionality to run on the board   
 *Logging data is put into the flash
 * Author - Tuan Anh Nguyen
 *
 * May 2018
 *------------------------------------------------------------
 */

#include "in4073.h"

#define MAX_ITEM 5450
#define LOG_ITEM_SIZE 24
//#define DEBUG

uint32_t logsize;
uint32_t current_address;

//Timestamp: 4 bytes; phi, theta, psi, sp, sq ,sr: 12 bytes.
//Motors: 8 bytes. Total = 24 bytes.
//Timestamp| phi theta psi| sp sq sr| motors|
uint8_t sensor_data[LOG_ITEM_SIZE] = {0};


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
    if (logsize > MAX_ITEM){
        printf("Flash memory full");
        return false;
    }
    uint32_t current_time = get_time_us();
	
    sensor_data[0] = MSBYTE_WORD(current_time);
	sensor_data[1] = BYTE2_WORD(current_time);
	sensor_data[2] = BYTE3_WORD(current_time);
	sensor_data[3] = LSBYTE_WORD(current_time);	

    //Attitude
    sensor_data[4] = MSBYTE(phi);
    sensor_data[5] = LSBYTE(phi);
    sensor_data[6] = MSBYTE(theta);
    sensor_data[7] = LSBYTE(theta);
    sensor_data[8] = MSBYTE(psi);
    sensor_data[9] = LSBYTE(psi);

    //Angular velocity
    sensor_data[10] = MSBYTE(sp);
    sensor_data[11] = LSBYTE(sp);
    sensor_data[12] = MSBYTE(sq);
    sensor_data[13] = LSBYTE(sq);
    sensor_data[14] = MSBYTE(sr);
    sensor_data[15] = LSBYTE(sr);

    for (int j = 0; j < 4; j++)
    {
        sensor_data[16 + 2*j] = MSBYTE( ae[j] );
        sensor_data[16 + 2*j + 1] = LSBYTE( ae[j] );
    }

    bool write_status = flash_write_bytes(current_address, sensor_data, LOG_ITEM_SIZE);
    if (write_status){
        logsize++;
        current_address += LOG_ITEM_SIZE;

        #ifdef DEBUG
        printf("Log successful.\n");
        #endif
	
        return true;
    } else {

        #ifdef DEBUG
        printf("Error writing to flash!");
        #endif
        return false;
    }
}

void log_read_last(){
    uint8_t buffer[LOG_ITEM_SIZE] = {0};
    uint32_t reading_address = current_address - LOG_ITEM_SIZE;
    printf("Reading log. Address %6ld.", reading_address);
    if(flash_read_bytes(reading_address, buffer,LOG_ITEM_SIZE)){
        for (int i = 0; i<LOG_ITEM_SIZE;i++){
            printf("%X ", buffer[i]);
        }
    }
}

void log_read_all(){
    uint8_t buffer[LOG_ITEM_SIZE] = {0};
    uint32_t reading_address = 0;
    printf(">>>>> Reading log. %6ld items. Size %6ld.",logsize,current_address);

    for (int i = 0; i<logsize; i++){
        if (flash_read_bytes(reading_address, buffer,24)){
            for(int count = 0; count <LOG_ITEM_SIZE;count++){
                printf("%X ", buffer[count]);
                buffer[count] = 0;
            }
            printf("\n");
            reading_address += LOG_ITEM_SIZE;
        }
    }

}