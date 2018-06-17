/*------------------------------------------------------------
 * telemtery_rx.c -> Parses and Process the telemetry data
 * received from drone  
 *
 * Author - Niket Agrawal
 *
 * May 2018
 *------------------------------------------------------------
 */
#include "pc_terminal.h"
#include "packet_constants.h"

//#define DEBUG
//#define DEBUGCRC

uint8_t packet_rx[TELEMETRY_PACKET_SIZE] = {0};  //char array of telemetry packet size
                                                 //to store a packet locally during processing
static uint8_t index_parser = 0;
static uint8_t crc = 0;
#ifdef DEBUGCRC
static int count = 0;
#endif
static uint8_t l = 0;  //variable used to determine new startbyte
uint16_t motor1 = 0;
uint16_t motor2 = 0;
uint16_t motor3 = 0;
uint16_t motor4 = 0;
// int16_t phi, theta, psi;
// int16_t sp, sq, sr = 0;
// int32_t pressure = 0;
uint16_t bat_volt;

uint16_t p_value;
uint8_t p1_value;
uint8_t p2_value;
int16_t srf;
int16_t sr;


int32_t timestamp = 0;

void process_telemetry(uint8_t c)
{
	if (index_parser == 0 && c != _STARTBYTE)
    {
        return ;
    }

    if (index_parser < TELEMETRY_PACKET_SIZE)
    {
        packet_rx[index_parser] = c;
        #ifdef DEBUGCRC
        //Corrupt CRC for random byte to simulate error scenario
        if((count == 1000) || (count == 1500) || (count == 2200)) {
            crc = 9;
         }
        else
        #endif
        crc = crc ^ c;

        index_parser = (index_parser + 1); // in any case, don't go over SIZEOFPACKET.    
    }

    if (index_parser == TELEMETRY_PACKET_SIZE) //reached the end of a packet 
    {
        #ifdef DEBUG
            printf("telemetry packet received~~~ : ");
            for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
            {
                printf("%X ", packet_rx[j]);
            }
            printf(" ~ crc = %X \n",crc); 
        #endif

        if (crc == 0) 
		{
            // Dsiplay telemtry data on PC (Author: Tuan Anh Nguyen)
            motor1 = (packet_rx[ROTOR1]<<8)|packet_rx[ROTOR1+1];
            motor2 = (packet_rx[ROTOR2]<<8)|packet_rx[ROTOR2+1];
            motor3 = (packet_rx[ROTOR3]<<8)|packet_rx[ROTOR3+1];
            motor4 = (packet_rx[ROTOR4]<<8)|packet_rx[ROTOR4+1];

            p_value = (packet_rx[P_YAW]<<8)|packet_rx[P_YAW+1];

            p1_value = packet_rx[P1]<<8|packet_rx[P1+1];
            p2_value = packet_rx[P2]<<8|packet_rx[P2+1];

            //phi = (packet_rx[PHI]<<8)|packet_rx[PHI+1];
            srf = (packet_rx[SRF]<<8)|packet_rx[SRF+1];
            sr = (packet_rx[SR]<<8)|packet_rx[SR+1];
            //setpoint_roll = (packet_rx[SETPOINT_ROLL]<<8)|packet_rx[SETPOINT_ROLL+1];

            bat_volt = (packet_rx[BAT_VOLT]<<8)|packet_rx[BAT_VOLT+1];
            timestamp = (packet_rx[TIMESTAMP]<<24)|(packet_rx[TIMESTAMP + 1]<<16)|(packet_rx[TIMESTAMP + 2]<<8)|packet_rx[TIMESTAMP+3];

            printf("Time |%6d|", timestamp);
            printf("Mode |%d|", packet_rx[MODE_DRONE]);
            printf("Motors |%3d %3d %3d %3d|", motor1,motor2,motor3,motor4);
            printf("P |%d|", p_value);
            printf("P1 |%d| ", p1_value);
            printf("P2 |%d|", p2_value);
            printf("srf |%d|", srf);

            //printf("setpoint_roll |%d|", setpoint_roll);
          
            // printf("|%6d %6d %6d|",phi,theta,psi);
            // printf("|%6d %6d %6d|",sp,sq,sr);
            printf("Battery |%4d|\n",bat_volt);
            //printf("|%6d| \n", pressure);
            if (packet_rx[MODE_DRONE] == 6){
                FILE *fp;
                fp = fopen("filtered_data.csv", "a");
                fprintf(fp, " %d, %d, %d\n", timestamp, sr, srf);
                fclose(fp);
            }
        }  
        else if (crc != 0)
        {
            #ifdef DEBUG
            // not passing. Pin-pon-error-blink-red
            printf(" - crc fail.");
            #endif

            crc = 0;  //reset crc as it will be recomputed
            l = l + 1;  // begin from the next byte after the previous start byte
            while(packet_rx[l] != _STARTBYTE)  //find the index of next startbyte starting 
            {                               //from the byte after the previous start byte
                //printf("iteration\n");
                l++;
            }      

            #ifdef DEBUGCRC                        
            printf("Next start byte found at %d\n", l);
            #endif

            if(l < TELEMETRY_PACKET_SIZE)
            {
                #ifdef DEBUGCRC
                printf("l is less than packet length\n");
                #endif
                //Compute crc from this new start byte till the last byte in our current array
                index_parser = 0;    //index is zero for new start byte, new packet starts from here   
                for(int i = l; i < TELEMETRY_PACKET_SIZE; i++)
                {
                    packet_rx[i-l] = packet_rx[i];  //overwrite previous bytes
                    crc = crc ^ packet_rx[i];
                    index_parser = index_parser + 1;
                }
                l = 0; //reset l to index 0 again
                //CRC computation done for the 1st half of the packet, new bytes arriving
                //to prpcess_packet method will form the 2nd half of this packet and CRC 
                //computation will continue from (index < size) check in line 33
            }

            else //no other start byte found in the current packet
                 //reset index to zero and clear packet 
            {
                index_parser = 0;
                for(int i = 0; i < TELEMETRY_PACKET_SIZE; i++)
                {
                    packet_rx[i] = 0;
                }
            }
            
        } 
        
    }

    // reset index to zero and clear packet if index reached the end of packet
    if(index_parser == TELEMETRY_PACKET_SIZE)
    {
        index_parser = 0;
        for(int i = 0; i < l; i++)  
        {
            packet_rx[i] = 0;
        }
    }
    #ifdef DEBUGCRC
    count++; 
    #endif
}