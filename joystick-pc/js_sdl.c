
#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>


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

int main (int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);              // Initialize SDL

	SDL_Surface *screen=NULL;
    SDL_Rect elem_gauge;
	SDL_Window *win = draw_window(screen);

    printf("%i joystick(s) were found.\n\n", SDL_NumJoysticks() );
    if (SDL_NumJoysticks() < 1)
        exit(EXIT_FAILURE);

    SDL_Joystick *joystick;

    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);

    SDL_Event event;
    /* Other initializtion code goes here */   

    /* Start main game loop here */
    char isContinuing = 1;
    int counter = 0;
    while(isContinuing)
    {
        while(SDL_PollEvent(&event))
        {  
            switch(event.type)
            {  
                case SDL_QUIT:
                isContinuing = 0;
                break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                /* handle keyboard stuff here */	
                switch (event.key.keysym.sym) {

                    case SDLK_ESCAPE: /* Appui sur la touche Echap, on arrête le programme */
                    isContinuing = 0;
                    break;

                    default:
                    printf("Key pressed: %c, status:%d, repeated:%d\n", event.key.keysym.sym, event.key.state, event.key.repeat);

                }
                break;

                case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */
                printf("Axis number %d, value %d\n",event.jaxis.axis, event.jaxis.value);
                break;

                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                printf("Button number %d, button state %d:\n",event.jbutton.button, SDL_JOYBUTTONUP - event.type);
                break;

                default:
                //printf("Some non-implemented event occured. Type: %d\n", event.type);
                break;
            }
        }
    }
    SDL_JoystickClose(joystick);
    SDL_DestroyWindow(win);
    // Clean up
    SDL_Quit();
    return 0;
}