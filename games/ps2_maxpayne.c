//===========================================================
// Mouse Injector for Dolphin
//==========================================================================
// Copyright (C) 2019-2020 Carnivorous
// All rights reserved.
//
// Mouse Injector is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, visit http://www.gnu.org/licenses/gpl-2.0.html
//==========================================================================
#include <stdint.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define PI 3.14159265f // 0x40490FDB

// pointers
#define MP_PLAYERBASE 0x66A1E8 //player base pointer
#define MP_CAMBASE 0x484	   //offset from playerbase
#define MP_ZOOMBASE 0x0066B988   //sniper zoom FOV pointer 

// States

// aim offsets from viewbase
#define MP_CAMY 0x20 //offset from cambase

//CAMX OFFSETS FROM PLAYERBASE
#define MP_CAMXSIN  0x54
#define MP_CAMXCOS  0x4C
#define MP_CAMXSINB  0x64
#define MP_CAMXCOSB  0x6C

#define MP_ZOOM 0x1C28 //offset from zoombase



static uint8_t PS2_MAXPAYNE_Status(void);
static void PS2_MAXPAYNE_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Max Payne",
		PS2_MAXPAYNE_Status,
		PS2_MAXPAYNE_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_MAXPAYNE = &GAMEDRIVER_INTERFACE;

static uint32_t playerBase = 0;
static uint32_t camBase = 0;
static uint32_t zoomBase = 0;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_MAXPAYNE_Status(void)
{
	// MAIN.RUN;1 (0x93390)
	return (PS2_MEM_ReadWord(0x93390) == 0x4D41494E&&
			PS2_MEM_ReadWord(0x93394) == 0x2E52554E&&
			PS2_MEM_ReadWord(0x93398) == 0x3B310000);
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_MAXPAYNE_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	   	return;

	playerBase = PS2_MEM_ReadPointer(MP_PLAYERBASE);
	camBase = PS2_MEM_ReadPointer(playerBase + MP_CAMBASE);
	zoomBase = PS2_MEM_ReadPointer(MP_ZOOMBASE);

	float looksensitivity = (float)sensitivity;
	float zoom = PS2_MEM_ReadFloat(zoomBase + MP_ZOOM); 
				 //PS2_MEM_ReadFloat(0xA482C8); 
				 //1; 


	float camY = PS2_MEM_ReadFloat(camBase + MP_CAMY);
		   
	float camXsin = PS2_MEM_ReadFloat(playerBase + MP_CAMXSIN);
	float camXcos = PS2_MEM_ReadFloat(playerBase + MP_CAMXCOS);
	float angle = atan(camXsin / camXcos);


	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * zoom / 15000.f;
	camY = ClampFloat(camY, -1.553343058f, 1.553343058f);

	if(camXcos < 0)
		angle -= PI;

	angle -= (float)xmouse * looksensitivity * zoom / 15000.f;

	camXsin = sin(angle);
	camXcos = cos(angle);

	PS2_MEM_WriteFloat(camBase + MP_CAMY, camY);

	PS2_MEM_WriteFloat(playerBase + MP_CAMXSIN, camXsin);
	PS2_MEM_WriteFloat(playerBase + MP_CAMXCOS, camXcos);

	//fixing body rotation animation
	PS2_MEM_WriteFloat(playerBase + MP_CAMXSINB, -camXsin);
	PS2_MEM_WriteFloat(playerBase + MP_CAMXCOSB, camXcos);

}