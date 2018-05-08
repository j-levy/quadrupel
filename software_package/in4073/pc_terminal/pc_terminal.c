/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include "pc_terminal.h"

#include "packet_constants.h"
#include "joystick.h"

// #define DEBUG
// #define DEBUGACK
// #define DEBUGCLK


uint8_t packet[SIZEOFPACKET] = {0};   		//Initializing packet to send
uint8_t previous_packet[SIZEOFPACKET] = {0};
unsigned short packet_id = 1; // start late to see the bug #7

uint8_t packet_rx[SIZEOFACKPACKET] = {0};  //packet received from Drone (an ACK packet for the moment)
static uint8_t rx_index = 0;  	//for prasing the received packets
static uint8_t rx_crc = 0; // not really needed but we can stay consistent with the board...
unsigned short rx_packet_id = 0;


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
 * Return value: '1' if packet ID matches the ID of the last sent packet
 *				 '0' otherwise
 *------------------------------------------------------------
 */
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


/*------------------------------------------------------------
 * Method to send the created packet over serial 
 *
 * Author - Niket Agrawal, Jonathan Lévy
 *
 * Check if dependency on 'size' info can be removed.
 *------------------------------------------------------------
 */

void send_packet(void* param)
{
	// Check ACK for the last sent packet

	if(packet_id == rx_packet_id+1)
	{
		//send new packet as usual
		packet[START] = _STARTBYTE;
		packet[PACKETID] = MSBYTE(packet_id);
		packet[PACKETID + 1] = LSBYTE(packet_id);
		packet_id++;
		uint8_t crc = 0;

		for(int i = 0; i < SIZEOFPACKET; i++)
		{
			crc = crc ^ packet[i];
		}
		packet[CRC] = crc;   
		int i = 0;
		while((rs232_putchar(packet[i]) == 1) && (i < SIZEOFPACKET))
		{
			i++;
		}

		// Store this packet
		memcpy(previous_packet, packet, SIZEOFPACKET);

		#ifdef DEBUG
			// display
			fprintf(stderr, "packet sent : ");
			for (int j = 0; j < SIZEOFPACKET; j++)
			{
				fprintf(stderr, "%X ", packet[j]);
			}
			fprintf(stderr, "\n");
		#endif

		//reset packet	
		for (int j = 0; j < SIZEOFPACKET; j++)
		{
			packet[j] = 0;
		}
	}
	else
	{
		
		int k = 0;
		//send previous packet again
		
		
		while((rs232_putchar(previous_packet[k]) == 1) && (k < SIZEOFPACKET))
		{
			k++;
		} 
		#ifdef DEBUG
			// display
			fprintf(stderr, "packet sent : ");
			for (int j = 0; j < SIZEOFPACKET; j++)
			{
				fprintf(stderr, "%X ", previous_packet[j]);
			}
			fprintf(stderr, "\n");
		#endif
				//reset packet	
		for (int j = 0; j < SIZEOFPACKET; j++)
		{
			packet[j] = 0;
		}
		
	}
}


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


	js_init(&fd, path_to_joystick);
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
			packet[AXISTHROTTLE + 2*j] = MSBYTE( jsdat->axis[j] );
			packet[AXISTHROTTLE + 2*j + 1] = LSBYTE( jsdat->axis[j] );
		}
		for (int j = 0; j < NBRBUTTONS; j++)
		{
			packet[JOYBUTTON] |= (jsdat->button[j] == 1) << j; // 10 for not storing anything.
		}
		
		if ((d = term_getchar_nb()) != -1)
		{
			if (d == 27) // escape key
				isContinuing = 0;

			packet[KEY] = d;
		}
		

		if ((rs232_getchar_nb(&c)) != -1)
		{
			term_putchar(c);
			process_rx(c);
		}

		
		clock_gettime(CLOCK_REALTIME, &tp);
		
		#ifdef DEBUGCLK
		fprintf(stderr, "clk=%ld,%ld\n",tp.tv_sec, tp.tv_nsec);
		#endif
		
		if (tp.tv_nsec - tic >= DELAY_PACKET_NS || tp.tv_sec - tic_s > 0)
		{
			tic = tp.tv_nsec;
			tic_s = tp.tv_sec;
			send_packet(NULL);
		}
		
	}

	JoystickData_destroy(jsdat);
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");


	return 0;
}

