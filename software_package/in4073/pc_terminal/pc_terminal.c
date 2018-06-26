/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 * 
 * Revised 
 * Niket Agrawal, Tuan Anh Nguyen 
 * May 2018
 * Methods for sending control packet to drone and
 * processing the telemetry data from drone 
 * Sending Telemetry
 * 
 * Jonathan Levy
 * Threads and interrupt for correct 
 *------------------------------------------------------------
 */

#include "pc_terminal.h"

#include "packet_constants.h"
#include "joystick.h"

// #define DEBUG
// #define DEBUGTIMEOUT
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


int 	rs232_putchar(char c)
{
	int result;

	do {
		result = (int) write(fd_RS232, &c, 1);
	} while (result == 0);

	assert(result == 1);
	return result;
}


// global variable, be read by both threads, but actually changed only by one thread (easier)
char isContinuing;
struct timespec tp;
uint8_t is_sending_packet, is_timeout;

// signal for quitting. Source: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/signal/install.html
void     INThandler(int);

void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     printf("Hitting CTRL+C. Attempt to quit normally.\n"
            "Expect <exit>...\n");
     isContinuing = 0;
}

void *entrythread_sender(void *param)
{
	while (isContinuing)
	{
		if (is_sending_packet)
		{
			send_packet();
			is_sending_packet = 0;
		}
	}
	return NULL;
}

void *entrythread_read(void *param)
{
		uint8_t c;

		while (isContinuing)
		{
			
			#ifdef DEBUGTIMEOUT
			if(count == 2000) //timeout failure scenario testcase 
			{
				//printf("Timeout!!\n");
				tic_rx = 100;
			}
			count++;
			#endif
			

			if ((rs232_getchar_nb(&c)) != -1)
		 	{
				if(is_timeout)
				{
					printf("timeout detected, sending mode as 1\n");
					control_packet[MODE] = '1';
					is_timeout = 0;
				}
				//term_putchar(c); 
				process_telemetry(c);
			}
		}
		return NULL;
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
	int d;
	int y ;
	int z ;
	#ifdef DEBUGTIMEOUT
	uint32_t count = 0;
	#endif
	
	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();

	term_puts("Type ^C to exit\n");

	js_init(&fd, path_to_joystick, &js);
	JoystickData *jsdat = JoystickData_create();

	clock_gettime(CLOCK_REALTIME, &tp);
	


	isContinuing = 1;
	signal(SIGINT, INThandler);

	/* 
	##################### CREATING THREADS #######################
	Jonathan Lévy
	freely inspired from: http://timmurphy.org/2010/05/04/pthreads-in-c-a-minimal-working-example/
	*/
	pthread_t thread0, thread1;
	int created_thread = pthread_create(&thread0, NULL, entrythread_read, NULL);
	if (created_thread)
	{
		printf("couldn't create thread 0!\n");
		exit(1);
	}
    created_thread = pthread_create(&thread1, NULL, entrythread_sender, NULL);
	if (created_thread)
	{
		printf("couldn't create thread 1!\n");
		exit(1);
	}

	long tic = tp.tv_nsec;
	time_t tic_s = tp.tv_sec;
	long tic_rx = tp.tv_nsec;

	while (isContinuing)
	{

		// Response time is a bit variable. Latency can be percieved still.
		clock_gettime(CLOCK_REALTIME, &tp);
		
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
		//If fire button is pressed send mode as panic (same as Esc key press behavior)
		if(control_packet[JOYBUTTON] == 1)
			control_packet[MODE] = 27;
		
		if ((d = term_getchar_nb()) != -1)
		{
			// Logic to read arrow key presses
			// Source: https://ubuntuforums.org/showthread.php?t=2276177
			if (d == 27) //escape key
			{
				y = getchar();
				z = getchar(); 

				if (d == 27 && y == 91)
				{
					switch (z)
					{
					case 65:   
					//up arrow key pressed
					control_packet[KEY] = 42;
					break;

					case 66:
					//down arrow key pressed
					control_packet[KEY] = 44;
					break;

					case 67:
					//right arrow key pressed
					control_packet[KEY] = 43;
					break;

					case 68:
					//left arrow key pressed
					control_packet[KEY] = 45;
					break;
					}
				}
				else
				control_packet[MODE] = 27; 
			}
			else if ((d >= '0') && (d <= '8'))
				control_packet[MODE] = d;
			else if (d == 3 || d == 16) // CTRL+C or CTRL+P
				isContinuing = 0;
			else
				control_packet[KEY] = d;


			printf("key | %c | pc_time | %d,%d", tp.tv_sec, tp.tv_nsec);
		}

		
		if ((tp.tv_nsec - tic >= DELAY_PACKET_NS || tp.tv_sec - tic_s > 0) && !is_sending_packet)
		{
			tic = tp.tv_nsec;
			tic_s = tp.tv_sec;
			is_sending_packet = 1;
		}

		if ((tp.tv_nsec - tic_rx) > TELEMETRY_TIMEOUT_NS && !is_timeout)
		{
			tic_rx = tp.tv_nsec;
			is_timeout = 1;
		}
							
					

	}
	pthread_join(thread0, NULL);
	pthread_join(thread1, NULL);

	JoystickData_destroy(jsdat);
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	return 0;
}

