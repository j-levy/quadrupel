#include "pc_terminal.h"

#include "joystick.h"

JoystickData *JoystickData_create() {
	JoystickData *jsdata = malloc(sizeof(JoystickData));
	for (int i = 0; i < NBRAXES; i++)
		(jsdata->axis)[i] = 0;
	for (int i = 0; i < NBRBUTTONS; i++)
		(jsdata->button)[i] = 0;
	return jsdata;
}

void JoystickData_destroy(JoystickData *jsdata)
{
	free(jsdata);
}

void js_init(int *fd, char *path_to_joystick, struct js_event *js)
{
	if ((*fd = open(path_to_joystick, O_RDONLY)) < 0) {
		perror("jserroropen");
		exit(1);
	}
	
	js->time = 0;
	js->value = 0;
	js->type = 0;
	js->number = 0;

	fcntl(*fd, F_SETFL, O_NONBLOCK);
}

void js_getJoystickValue(int *fd, struct js_event *js, JoystickData *jsdat)
{
		/* check up on JS
		 */
		while (read(*fd, js, sizeof(struct js_event)) == 
		       			sizeof(struct js_event))  {

			/* register data
			 */
			// fprintf(stderr,".");
			switch(js->type & ~JS_EVENT_INIT) {
				case JS_EVENT_BUTTON:
					jsdat->button[js->number] = js->value;
					break;
				case JS_EVENT_AXIS:
					//fprintf(stderr, "reading joystick\n");
					jsdat->axis[js->number] = js->value;
					break;
			}
		}
		
		if (errno != EAGAIN) {
			perror("\njs: error reading (EAGAIN)");
			exit (1);
		}
		

}
