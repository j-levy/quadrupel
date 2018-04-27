
#include <stdlib.h>
#include <stdio.h>

#define LINUX

#ifdef LINUX
    #include "SDL.h"
#endif

SDL_Window *draw_window(SDL_Surface *screen){
	SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);              // Initialize SDL

    // Create an application window with the following settings:
        window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );
    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return (SDL_Window*) NULL;
    }

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return (SDL_Window*) NULL;
    }

    // The window is open: could enter program loop here (see SDL_PollEvent())

    //SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example
	return window;
}

int destroy_window(SDL_Window *window){
	// Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
}

int main (int argc, char **argv)
{

	SDL_Surface *screen=NULL;
    SDL_Rect elem_gauge;
	SDL_Window *win = draw_window(screen);

    printf("%i joysticks were found.\n\n", SDL_NumJoysticks() );

    SDL_Joystick *joystick;

    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);


    SDL_Event event;
    /* Other initializtion code goes here */   

    /* Start main game loop here */
    while(1)
    {
        while(SDL_PollEvent(&event))
        {  
            switch(event.type)
            {  
                case SDL_KEYDOWN:
                /* handle keyboard stuff here */	


                break;

                case SDL_QUIT:
                /* Set whatever flags are necessary to */
                /* end the main game loop here */
                destroy_window(win);
                return 0;
                break;

                case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */
                printf("Axis number %d, value %d\n",event.jaxis.axis, event.jaxis.value);
                break;

                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                printf("Button number %d, button state %d:\n",event.jbutton.button, event.type - SDL_JOYBUTTONDOWN);
                break;
            }
        }
    }
    return 0;
}