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
#include "../joystick.h"
#include "game.h"

#define PI 3.14159265f // 0x40490FDB

// pointers
#define SC_PLAYERBASE 0x49AD08	 //pointer to many things
#define SC_CAMBASE 0x34	 //offset from playerbase

//offsets from camBase
#define SC_CAMY 0xF0
#define SC_CAMX 0xF4
#define SC_fov 0x308 //Field of view

static uint8_t PS2_SC_Status(void);
static void PS2_SC_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Splinter Cell",
		PS2_SC_Status,
		PS2_SC_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_SPLINTERCELL = &GAMEDRIVER_INTERFACE;

static uint32_t playerBase = 0;
static uint32_t camBase = 0;
static float xAccumulator = 0.f;
static float yAccumulator = 0.f;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_SC_Status(void)
{
	// SLUS_203.74; (0x93390)
	// LOADER.PS2;1


	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323036&&
			PS2_MEM_ReadWord(0x93398) == 0x2E35323B)||
					
			(PS2_MEM_ReadWord(0x93390) == 0x4C4F4144&&
			PS2_MEM_ReadWord(0x93394) == 0x45522E50&&
			PS2_MEM_ReadWord(0x93398) == 0x53323B31));		  
			  
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_SC_Inject(void)
{

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	int16_t scale = 2;
	
	playerBase = PS2_MEM_ReadPointer(SC_PLAYERBASE);
	camBase = PS2_MEM_ReadPointer(playerBase + SC_CAMBASE);

	float looksensitivity = (float)sensitivity;
	float fov = PS2_MEM_ReadFloat(camBase + SC_fov) / 70.f;

	// while not aiming
	int16_t camY = PS2_MEM_ReadInt16(camBase + SC_CAMY);
	int16_t camX = PS2_MEM_ReadInt16(camBase + SC_CAMX);
	float camYf = (float)camY;
	float camXf = (float)camX;

	//update cam
	float ym = (float)(invertpitch ? ymouse : -ymouse);
	float dy = ym * looksensitivity * fov / scale;
	AccumulateAddRemainder(&camYf, &yAccumulator, ym, dy);
	float dx = (float)xmouse * looksensitivity * fov / scale;
	AccumulateAddRemainder(&camXf, &xAccumulator, (float)xmouse, dx);

	//write cam
	PS2_MEM_WriteInt16(camBase + SC_CAMY, (int16_t)camYf);
	PS2_MEM_WriteInt16(camBase + SC_CAMX, (int16_t)camXf);
}