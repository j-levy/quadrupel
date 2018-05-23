/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 * 
 * Revised 
 * [Niket Agrawal]
 * May 2018
 * Methods for sending control packet to drone and
 * processing the telemetry data from drone 
 *------------------------------------------------------------
 */

#include "pc_terminal.h"

#include "packet_constants.h"
#include "joystick.h"

// #define DEBUG
// #define DEBUGACK
// #define DEBUGCLK


//uint8_t control_packet[CONTROL_PACKET_SIZE] = {0};   		//Initializing packet to send

//uint8_t packet_rx[TELEMETRY_PACKET_SIZE] = {0};  //packet received from Drone

// static uint8_t index_parser = 0;
// static uint8_t crc = 0;
// #ifdef DEBUGCRC
// static int count = 0;
// #endif
// static uint8_t l = 0;  //variable used to determine new startbyte

// static uint8_t rx_index = 0;  	//for prasing the received packets
// static uint8_t rx_crc = 0; // not really needed but we can stay consistent with the board...

/* useful values for joystick*/
int 		fd;
struct js_event js;



/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
struct termios 	savetty;

void	term_initio()
{
	struct termios tty;

	tcgetattr(0, &savetty);
	tcgetattr(0, &tty);

	tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;

	tcsetattr(0, TCSADRAIN, &tty);
}

void	term_exitio()
{
	tcsetattr(0, TCSADRAIN, &savetty);
}

void	term_puts(char *s)
{
	fprintf(stderr,"%s",s);
}

void	term_putchar(char c)
{
	putc(c,stderr);
}

int	term_getchar_nb()
{
        static unsigned char 	line [2];

        if (read(0,line,1)) // note: destructive read
        		return (int) line[0];

        return -1;
}

int	term_getchar()
{
        int    c;

        while ((c = term_getchar_nb()) == -1)
                ;
        return c;
}

/*------------------------------------------------------------
 * Serial I/O
 * 8 bits, 1 stopbit, no parity,
 * 115,200 baud
 *------------------------------------------------------------
 */

int serial_device = 0;
int fd_RS232;

void rs232_open(void)
{
  	char 		*name;
  	int 		result;
  	struct termios	tty;

	fd_RS232 = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);  // Hardcode your serial port here, or request it as an argument at runtime

	assert(fd_RS232>=0);

  	result = isatty(fd_RS232);
  	assert(result == 1);

  	name = ttyname(fd_RS232);
  	assert(name != 0);

  	result = tcgetattr(fd_RS232, &tty);
	assert(result == 0);

	tty.c_iflag = IGNBRK; /* ignore break condition */
	tty.c_oflag = 0;
	tty.c_lflag = 0;

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
	tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */

	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 1; // added timeout

	tty.c_iflag &= ~(IXON|IXOFF|IXANY);

	result = tcsetattr (fd_RS232, TCSANOW, &tty); /* non-canonical */

	tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}


void 	rs232_close(void)
{
  	int 	result;

  	result = close(fd_RS232);
  	assert (result==0);
}

// modified to distinguish error code and character read.
// The character read is passed as pointer and modified that way.
int	rs232_getchar_nb(uint8_t *c)
{
	int 		result;

	result = read(fd_RS232, c, 1);

	if (result == 0)
		return -1;

	else
	{
		assert(result == 1);
		
		return 1;
	}
}


/* This function is not used, and is shit anyways. Needs to be re-written with the new parameter to 
	pass. Who would need that anyways.*/
/*
uint8_t 	rs232_getchar()
{
	int 	c;

	while ((rs232_getchar_nb(&c)) == -1)
		;
	return c;
}
*/


int 	rs232_putchar(char c)
{
	int result;

	do {
		result = (int) write(fd_RS232, &c, 1);
	} while (result == 0);

	assert(result == 1);
	return result;
}


/*------------------------------------------------------------
 * Method to process the received ACK packet from Drone 
 *
 * Author - Niket Agrawal
 *
 * Compares the received packet ID with the ID of the 
 * last sent packet.
 * 
 *------------------------------------------------------------
 
void process_rx(uint8_t c)
{
		//fprintf(stderr, "read: %X", c);
		if (rx_index == 0 && c != _STARTBYTE)
    	{
        	return ;
    	}

    	if (rx_index < SIZEOFACKPACKET)
    	{
        	packet_rx[rx_index] = c;  //to print the received packet contents 
        	rx_crc = rx_crc ^ c;
        	rx_index = rx_index + 1; 
    	}

    	if (rx_index == SIZEOFACKPACKET) // whole packet received
    	{		
    		#ifdef DEBUGACK
        		fprintf(stderr, "packet got~~~ : ");
            	for (int j = 0; j < SIZEOFACKPACKET; j++)
            	{
                	fprintf(stderr, "%X ", packet_rx[j]);
            	}
        		fprintf(stderr, " ~ crc = %X\n",rx_crc); 
        	#endif

    		if (rx_crc == 0)  //CRC check passed
    		{
    			//compare the packet ID in the ACK packet with 
    			//the packet ID of the last sent packet
    			rx_packet_id = ((uint16_t) packet_rx[PACKETID])*256 + packet_rx[PACKETID+1]; /// They're the same offset as the big packet

				fprintf(stderr, "rx_packet_id = %d\n", rx_packet_id);

				// the rx_packet_id is updated, and the send_packet will be able to read it.


    		}
    		else
    		{
    			fprintf(stderr, "CRC check failed for the ACK packet\n");
    		}
			rx_index = 0;
			rx_crc = 0;
    	}
	
}
*/



/*------------------------------------------------------------
 * Method to send the created packet over serial 
 *
 * Author - Niket Agrawal
 *
 *------------------------------------------------------------
 *

void send_packet()
{
	control_packet[START] = _STARTBYTE;

	uint8_t crc = 0;

	for(int i = 0; i < CONTROL_PACKET_SIZE; i++)
	{
		crc = crc ^ control_packet[i];
	}
	control_packet[CRC] = crc;   
	int i = 0;

	while((rs232_putchar(control_packet[i]) == 1) && (i < CONTROL_PACKET_SIZE))
	{
		i++;
	}

	#ifdef DEBUG
		// display the packet that is sent
		//fprintf(stderr, "control packet sent : ");
		for (int j = 0; j < CONTROL_PACKET_SIZE; j++)
		{
			fprintf(stderr, "%X ", control_packet[j]);
		}
		fprintf(stderr, "\n");
	#endif

	//reset packet	
	for (int j = 0; j < CONTROL_PACKET_SIZE; j++)
	{
		if(j != MODE)  //Retain the previous mode info
			control_packet[j] = 0;	
	}
	
}
*/


/*-----------------------------------------------------------------
* Function to read arrow key inputs
* Source: https://ubuntuforums.org/showthread.php?t=2276177
*------------------------------------------------------------------
*/
// int getch()
// {
//  int ch;
//  struct termios oldt;
//  struct termios newt;
//  tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
//  newt = oldt; /* copy old settings to new settings */
//  newt.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
//  tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediatly */
//  ch = getchar(); /* standard getchar call */
//  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */
//  return ch; /*return received char */
// }

/*------------------------------------------------------------
 * Method to process the received telemetry data from drone 
 *
 * Author - Niket Agrawal
 *
 *------------------------------------------------------------
 *
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
        if((count == 750) || (count == 150) || (count == 1300)) {
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
            printf("packet received~~~ : ");
            for (int j = 0; j < TELEMETRY_PACKET_SIZE; j++)
            {
                printf("%X ", packet_rx[j]);
            }
            printf(" ~ crc = %X \n",crc); 
        #endif

        if (crc == 0) 
		{
            // last_OK[0] = packet[PACKETID];
            // last_OK[1] = packet[PACKETID+1];
            // #ifdef DEBUG

            // send_ack(packet[PACKETID], packet[PACKETID+1]);

            // #endif
            //l = CONTROL_PACKET_SIZE;
            // #ifndef DEBUG 

            // // send_ack(packet[PACKETID], packet[PACKETID+1]);

            // process_key(packet + KEY);

            // process_joystick_axis(packet + AXISTHROTTLE); // throttle is the first value.

            // process_joystick_button(packet + JOYBUTTON);

            // #endif

        }  
        else if (crc != 0)
        {
            #ifdef DEBUG
            // not passing. Pin-pon-error-blink-red
            printf(" - crc fail.");
            nrf_gpio_pin_toggle(RED);
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

            if(l < CONTROL_PACKET_SIZE)
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
                //packet_length = l + CONTROL_PACKET_SIZE;  //reset packet length for full packet traversal
                //printf("New packet length is %d\n", packet_length);
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
*/


/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	char path_to_joystick[64];
	if (argc == 1) {
		strcpy(path_to_joystick, "/dev/input/js0");
	} else if (argc == 2) {
		strcpy(path_to_joystick, argv[1]);
	} else {
		fprintf(stderr, "too many arguments. 0 argument = default path (js0), 1 argument = path to joystick");
		return 1;
	}
	fprintf(stderr, "using %s\n", path_to_joystick);
	uint8_t c;
	int d;
	int y ;
	int z ;
	
	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	/*
	while ((rs232_getchar_nb(&c) != -1)
		fputc(c,stderr);
	*/
	/* send & receive
	 */

	char isContinuing = 1;


	js_init(&fd, path_to_joystick, &js);
	JoystickData *jsdat = JoystickData_create();

	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	
	long tic = tp.tv_nsec;
	time_t tic_s = tp.tv_sec;

	while (tp.tv_sec - tic_s < 3)
	{
		clock_gettime(CLOCK_REALTIME, &tp);
	}
		

	while (isContinuing)
	{
		// Response time is a bit variable. Latency can be percieved still.

		// poll axes with raw-OS method
		js_getJoystickValue(&fd, &js, jsdat);
		for (int j = 0; j < NBRAXES; j++)
		{
			control_packet[AXISROLL + 2*j] = MSBYTE( jsdat->axis[j] );
			control_packet[AXISROLL + 2*j + 1] = LSBYTE( jsdat->axis[j] );
			//control_packet[AXISTHROTTLE + 2*j] = 0xFF;
			//control_packet[AXISTHROTTLE + 2*j + 1] = 0xFF;
		}
		for (int j = 0; j < NBRBUTTONS; j++)
		{
			control_packet[JOYBUTTON] |= (jsdat->button[j] == 1) << j; // 10 for not storing anything.
		}
		
		if ((d = term_getchar_nb()) != -1)
		{
			// Logic to read arrow key presses
			// Source: https://ubuntuforums.org/showthread.php?t=2276177
			if (d == 27) //escape key
			{
				y = getchar();
  				z = getchar(); 
  				//printf("Key code y is %d\n", y);
  				//printf("Key code z is %d\n", z);
				if (d == 27 && y == 91)
 				{
  					switch (z)
  					{
   					case 65:   
   					//printf("up arrow key pressed\n");
					control_packet[KEY] = 42;
   					break;

   					case 66:
   					//printf("down arrow key pressed\n");
					control_packet[KEY] = 44;
   					break;

   					case 67:
   					//printf("right arrow key pressed\n");
					control_packet[KEY] = 43;
   					break;

   					case 68:
   					//printf("left arrow key pressed\n");
					control_packet[KEY] = 45;
   					break;
  					}
 				}
				else 
				isContinuing = 0;
			}
			else if ((d >= 48) && (d <= 56))
				control_packet[MODE] = d;
				//control_packet[MODE] = 0xFF;
			else 
				control_packet[KEY] = d;
				//control_packet[KEY] = 0xFF;
		}

		 if ((rs232_getchar_nb(&c)) != -1)
		 {
		 	//term_putchar(c);
		  	process_telemetry(c);
		 }

		clock_gettime(CLOCK_REALTIME, &tp);
		
		#ifdef DEBUGCLK
		fprintf(stderr, "clk=%ld,%ld\n",tp.tv_sec, tp.tv_nsec);
		#endif
		
		if (tp.tv_nsec - tic >= DELAY_PACKET_NS || tp.tv_sec - tic_s > 0)
		{
			tic = tp.tv_nsec;
			tic_s = tp.tv_sec;
			send_packet();
		}
	}

	JoystickData_destroy(jsdat);
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}

