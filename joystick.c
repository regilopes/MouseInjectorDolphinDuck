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
//static POINT mouselock;				 // center screen X and Y var for mouse
static uint8_t lockmousecounter = 0; // limit SetCursorPos execution

uint8_t JOYSTICK_Init()
{
	// SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_JOYSTICK);
	// SDL_JoystickEventState(SDL_ENABLE);
	//  if(SDL_NumJoysticks() < 1){
	//  	return TRUE;
	//  };
	// SDL_SetHintWithPriority(SDL_HINT_WINDOWS_FORCE_MUTEX_CRITICAL_SECTIONS, "1", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_THREAD_FORCE_REALTIME_TIME_CRITICAL, "1", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_DIRECTINPUT_ENABLED,"0", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_XINPUT_ENABLED, "0", SDL_HINT_OVERRIDE);
	//  Initialize SDL and the joystick subsystem
	SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1");
	SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1", SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_RAWINPUT, "0",SDL_HINT_OVERRIDE);

	// SDL_SetHint(SDL_HINT_WINDOWS_FORCE_SEMAPHORE_KERNEL, "1");
	// SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "0");
	// SDL_SetHint(SDL_HINT_WINDOWS_FORCE_MUTEX_CRITICAL_SECTIONS, "1");

	// SDL_SetHint(SDL_HINT_VIDEODRIVER, "offscreen");

	// SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_WGI, "0",SDL_HINT_OVERRIDE);

	// SDL_SetHintWithPriority(SDL_HINT_POLL_SENTINEL, "1",SDL_HINT_OVERRIDE);
	// SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_RAWINPUT_CORRELATE_XINPUT, "1",SDL_HINT_OVERRIDE);

	// SDL_SetHintWithPriority(SDL_HINT_ALLOW_TOPMOST, "0",SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1", SDL_HINT_OVERRIDE);

	if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
	{
		fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return 1;
	}
	printf("SDL initialized successfully.\n");

	// window = SDL_CreateWindow("PUTA QUIU PARIU", 1920, 1080, 1, 1, //SDL_WINDOW_INPUT_GRABBED 		|
	// 															   //SDL_WINDOW_SHOWN					|
	// 															   //SDL_WINDOW_UTILITY					|
	// 															   //SDL_WINDOW_HIDDEN				|
	// 															   //SDL_WINDOW_INPUT_FOCUS 			|
	// 															   SDL_WINDOW_ALWAYS_ON_TOP			|
	// 															   SDL_WINDOW_BORDERLESS 			|
	// 															   //SDL_WINDOW_MINIMIZED 			|
	// 															   SDL_WINDOW_MAXIMIZED
	// 															   );

	controller = SDL_GameControllerOpen(0);
}

void JOYSTICK_Quit(void)
{
	SDL_Quit();
}

// void JOYSTICK_Lock(void)
// {
// 	GetCursorPos(&mouselock);
// }

void JOYSTICK_Update(const uint16_t tickrate)
{

	// if (tickrate > 8)							// if game driver tickrate is over 8ms, do not bother limiting SetCursorPos calls
	// 	SetCursorPos(mouselock.x, mouselock.y); // set mouse position back to lock position
	// else
	// {
	// 	if (lockmousecounter % 25 == 0)				// don't execute every tick
	// 		SetCursorPos(mouselock.x, mouselock.y); // set mouse position back to lock position
	// 	lockmousecounter++;							// overflow pseudo-counter
	// }

	//rx = ry = 0; // reset joystick input

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_CONTROLLERAXISMOTION)
		{
			// The joystick axis has moved
			// printf("%s %d axis %d moved to value: %d\n", SDL_GameControllerName(controller), event->caxis.which, event->caxis.axis, event->caxis.value);

			if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
				if (event.caxis.value > 100 || event.caxis.value < -100)
					rx = event.caxis.value;
				else
					rx = 0;

			if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
				if (event.caxis.value > 100 || event.caxis.value < -100)
					ry = event.caxis.value;
				else
					ry = 0;
		}
	}
}
