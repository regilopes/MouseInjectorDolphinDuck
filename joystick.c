#include <SDL2/SDL.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

int16_t rx, ry;
int32_t smousex, smousey;
SDL_Event event;
int quit = 0;
static SDL_GameController *controller = NULL;
static SDL_Window *window = NULL;
static uint8_t lockmousecounter = 0; 

uint8_t JOYSTICK_Init() {
    SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1");
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1", SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1", SDL_HINT_OVERRIDE);
    
    // Initializes the SDL subsystems for game controllers and events
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    //printf("SDL initialized successfully.\n");

    // Opens the first available controller, if there is one connected at startup
    if (SDL_NumJoysticks() > 0 && SDL_IsGameController(0)) {
        controller = SDL_GameControllerOpen(0);
        if (controller) {
            //printf("Controller connected: %s\n", SDL_GameControllerName(controller));
        }
    }
    return 0;
}

void JOYSTICK_Quit(void) {
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = NULL;
    }
    SDL_Quit();
}

void JOYSTICK_Update(const uint16_t tickrate) {
    SDL_ShowCursor(SDL_FALSE);
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            // Detects when a new controller is plugged in
            case SDL_CONTROLLERDEVICEADDED:
                if (!controller) { // Se não houver controle ativo, abre o novo
                    controller = SDL_GameControllerOpen(event.cdevice.which);
                    if (controller) {
                        //printf("New controller connected: %s\n", SDL_GameControllerName(controller));
                    }
                }
                break;

            // Detects when the current controller is unplugged
            case SDL_CONTROLLERDEVICEREMOVED:
                if (controller) {
                    SDL_Joystick *joystick = SDL_GameControllerGetJoystick(controller);
                    SDL_JoystickID instance_id = SDL_JoystickInstanceID(joystick);
                    
                    // Verifies if the removed controller is the one we are using
                    if (event.cdevice.which == instance_id) {
                        SDL_GameControllerClose(controller);
                        controller = NULL;
                        rx = 0;
                        ry = 0;
                        //printf("Controller disconnected.\n");
                    }
                }
                break;

            // Reads the right analog stick's X and Y axis values
            case SDL_CONTROLLERAXISMOTION:
                if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
                    if (event.caxis.value > 2500 || event.caxis.value < -2500) 
                        rx = event.caxis.value;
                    else 
                        rx = 0;
                }
                if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
                    if (event.caxis.value > 2500 || event.caxis.value < -2500) 
                        ry = event.caxis.value;
                    else 
                        ry = 0;
                }
                break;
        }
    }
}
