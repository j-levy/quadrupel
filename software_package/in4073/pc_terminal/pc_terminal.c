/*------------------------------------------------------------
 * Simple pc terminal in C
 *
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "packet_constants.h"

#include <SDL.h>



#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

char packet[SIZEOFPACKET] = {0};   		//Initializing packet to send to the drone over serial 

SDL_Window *draw_window(SDL_Surface *screen){
	SDL_Window *window;                    // Declare a pointer
    // Create an application window with the following settings:
        window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        300,                               // width, in pixels
        200,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );
    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // The window is open: could enter program loop here (see SDL_PollEvent())

    //SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example
	return window;
}



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
#include <termios.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

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


int	rs232_getchar_nb()
{
	int 		result;
	unsigned char 	c;

	result = read(fd_RS232, &c, 1);

	if (result == 0)
		return -1;

	else
	{
		assert(result == 1);
		return (int) c;
	}
}


int 	rs232_getchar()
{
	int 	c;

	while ((c = rs232_getchar_nb()) == -1)
		;
	return c;
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


/*------------------------------------------------------------
 * Method to create packet as per the protocol to be sent over serial 
 *
 * Author - Niket Agrawal
 *
 * (Full protocol details to be updated here)
 * Start bit - 0x01 (1 byte)
 * Packet type - Mode packet:0, Joystick packet:1, trimming packet:2  (1 byte)
 * 
 * Returns pointer to newly created packet
 *------------------------------------------------------------
 
char *create_packet(int keyboard_char, int joy_button_index, int joy_stick_index, int joy_stick_value)
{
	char packet[14];
	packet[0] = 1;   	//start byte

	packet[1] = (packet_id & 0xff00) >> 8; 	//packet ID
	packet[2] = value & 0x00ff;

	packet[3] = keyboard_char;
	packet[4] = joy_button_index;

	 
	packet[5] = (joy_stick_value & 0xff00) >> 8;
	packet[6] = joy_stick_value & 0x00ff;
	
	
	packet[1] = type;   // type of packet
	printf("type in create_packet function is %d\n", type);

	
	switch(type)
	{
		case 0: 
				printf("mode data from keyboard\n");
				packet[2] = value;         //type zero detected -> mode data from keyboard received
		break;

		case 1: 
				printf("joystick data detected\n");
				packet[2] = index_state;  // type 1 detected -> data from joystick -> 
										  //this byte holds the stick/button index
										  // two bytes needed to hold the stick 
										  //value(16 bit signed) or button state(0/1)
				packet[3] = (value & 0xff00) >> 8; //storing MSB first
				packet[4] = value & 0x00ff;
				packet[5] = packet[0] ^ packet[1] ^ packet[2] ^ packet[3] ^ packet[4]; //CRC
		break; 

		case 2: 
				printf("trimming data detected\n");
				packet[2] = value;   //type 2 detected -> static offset received from keyboard
		break;

		default: 
				printf("Invalid input\n");
		break;
	}
	

	char *return_packet = (char *)malloc(14 * sizeof(char));
	memcpy(return_packet, packet, 14);
	
	if(packet_id < 65535)
	{
		packet_id++;		
	}
	else
	{
		packet_id = 1;
	}
	return return_packet;
}
*/

/*------------------------------------------------------------
 * Method to send the created packet over serial 
 *
 * Author - Niket Agrawal
 *
 * Check if dependency on 'size' info can be removed.
 *------------------------------------------------------------
 */
void send_packet(void* param)
{
	int i = 0;
	while((rs232_putchar(packet[i]) == 1) && (i < SIZEOFPACKET))
	{
		i++;
	}

	printf("packet sent\n");

	memset(packet, 0, sizeof(packet));	
}

Uint32 callback_send_packet(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    /* In this example, our callback pushes a function
    into the queue, and causes our callback to be called again at the
    same interval: */

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = &send_packet;
    userevent.data2 = param;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(interval);
}

void (*p) (void*);


/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	unsigned short packet_id = 1;
	
	char crc = 0;
	packet[START] = 0xff;
	
	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	term_initio();
	rs232_open();

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	/*
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);
	*/
	/* send & receive
	 */

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER);              // Initialize SDL

	SDL_Surface *screen=NULL;
	SDL_Window *win = draw_window(screen);

    printf("%i joystick(s) were found.\n\n", SDL_NumJoysticks() );
    if (SDL_NumJoysticks() < 1)
        exit(EXIT_FAILURE);


    SDL_TimerID my_timer_id = SDL_AddTimer(DELAYSENDPACKET, callback_send_packet, NULL);
    if (my_timer_id == 0)
    {
    	SDL_GetError();
    	exit (EXIT_FAILURE);
    }


    SDL_JoystickEventState(SDL_ENABLE);
	SDL_Joystick *joystick;
    joystick = SDL_JoystickOpen(0);

	SDL_Event event;
	char isContinuing = 1;


	while (isContinuing)
	{
		
		

		packet[PACKETID] = MSBYTE(packet_id); 	//packet ID
		packet[PACKETID + 1] = LSBYTE(packet_id);

		// poll axes values
		short axisvalue = 0;
		for (int i = 0; i < 4; i++)
		{
			axisvalue = SDL_JoystickGetAxis(joystick, i);  //To keep sending packets at the stick extremes 
			packet[AXISTHROTTLE + 2*i] = MSBYTE(axisvalue);
			packet[AXISTHROTTLE + 2*i +1] = LSBYTE(axisvalue);

		}

		// poll for keyboard or buttons event
		if(SDL_PollEvent(&event))
        {  
            switch(event.type)
            {  
                case SDL_QUIT:
				isContinuing = 0;
                break;
                
                
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                // handle keyboard stuff here 	
                switch (event.key.keysym.sym) {

                    case SDLK_ESCAPE: 
					isContinuing = 0;
                    break;

                    default:
                    printf("Key pressed: %c, status:%d, repeated:%d\n", event.key.keysym.sym, event.key.state, event.key.repeat);
                    
                    if(((event.key.keysym.sym >= '0') && (event.key.keysym.sym <= '8' )) && (event.key.state == 1))
                
                    {
                    	printf("========Mode input from keyboard detected=========\n");
                    	packet[KEY] = event.key.keysym.sym;
                    	break;
                    }
                    else if((((event.key.keysym.sym >= 'a') && (event.key.keysym.sym <= 'z' )) && (event.key.state == 1))  
                    	&& !((packet[KEY] >= '0') && (packet[KEY] <= '8')))
                    {
                    	printf("=======Trimming data input from keyboard detected=========\n");
                    	packet[KEY] = event.key.keysym.sym;
                    	break;
                    }
                }
                break;
				
				/*
                case SDL_JOYAXISMOTION:  // Handle Joystick Motion /
                printf("Axis number %d, value %d\n",event.jaxis.axis, event.jaxis.value);
                if(!event.jaxis.value)
                {
                	switch(event.jaxis.axis)
                	{
                		case '0':
                		printf("Axis zero detected\n");
                		packet[AXISTHROTTLE + event.jaxis.axis]	= (joy_stick_value & 0xff00) >> 8;
                		packet[AXISTHROTTLE + event.jaxis.axis +1]	= (joy_stick_value & 0x00ff);
                		break;

                		case '1':
                		printf("Axis 1 detected\n");
                		packet[AXISROLL + event.jaxis.axis]	= (joy_stick_value & 0xff00) >> 8;
                		packet[AXISROLL + event.jaxis.axis +1]	= (joy_stick_value & 0x00ff);
                		break;

                		case '2':
                		printf("Axis 2 detected\n");
                		packet[AXISPITCH + event.jaxis.axis]	= (joy_stick_value & 0xff00) >> 8;
                		packet[AXISPITCH + event.jaxis.axis +1]	= (joy_stick_value & 0x00ff);
                		break;

                		case '3':
                		printf("Axis 3 detected\n");
                		packet[AXISYAW + event.jaxis.axis]	= (joy_stick_value & 0xff00) >> 8;
                		packet[AXISYAW + event.jaxis.axis +1]	= (joy_stick_value & 0x00ff);
                		break;

                		default:
                		printf("Invalid stick axis detected\n");
                	}
                	
                }
                break;
                */

                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:                
                printf("Button number %d, button state %d:\n",event.jbutton.button, event.jbutton.state);
                if(event.jbutton.state)
                {
                	packet[JOYBUTTON] |= 0xff & (1 << event.jbutton.button);		
                }
                break;


                case SDL_USEREVENT:
					p = event.user.data1;
            		p(event.user.data2);
                break;

                default:
                //printf("Some non-implemented event occured. Type: %d\n", event.type);
                break;
            }
        }
		

/*
		if ((c = term_getchar_nb()) != -1)
			rs232_putchar(c);

		if ((c = rs232_getchar_nb()) != -1)
			term_putchar(c);
*/		
        crc = packet[0] ^ packet[1];

        for(int i = 2; i < 13; i++)
        {
        	crc = crc ^ packet[i];
        }
        packet[CRC] = crc;        

        

       
        
	}

	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");

	SDL_JoystickClose(joystick);
    SDL_DestroyWindow(win);
    // Clean up
    SDL_Quit();

	return 0;
}

