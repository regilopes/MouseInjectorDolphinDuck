#include <stdint.h>
#include <windows.h>
#include "gamepad.h"
#include <SDL2/SDL.h>
//#include "./include/SDL2/SDL_gamecontroller.h"

uint8_t GAMEPAD_Init(void);
void GAMEPAD_Quit(void);
void GAMEPAD_Update(const uint16_t tickrate);


uint8_t GAMEPAD_Init(void)
{
	return;
    SDL_Init(SDL_INIT_GAMECONTROLLER);
}

void GAMEPAD_Quit(void)
{
	ManyMouse_Quit();
}