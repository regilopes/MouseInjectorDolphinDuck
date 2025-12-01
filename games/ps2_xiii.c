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
#define XIII_PLAYERBASE 0x437AF8 //player base pointer
#define XIII_AIMBASE 0x30	     //offset from playerbase

// States

// aim offsets from aimbase
#define XIII_AIMY 0xD8//offset from cambase
#define XIII_AIMX  0xDC


#define XIII_ZOOM 0x1F8 //offset from aimbase



static uint8_t PS2_XIII_Status(void);
static void PS2_XIII_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"XIII",
		PS2_XIII_Status,
		PS2_XIII_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_XIII = &GAMEDRIVER_INTERFACE;

static uint32_t playerBase = 0;
static uint32_t aimBase = 0;
static float xAccumulator = 0.f;
static float yAccumulator = 0.f;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_XIII_Status(void)
{
	// SLUS_206.77 (0x155D0)
	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323036&&
			PS2_MEM_ReadWord(0x93398) == 0x2E37373B)||
			
			(PS2_MEM_ReadWord(0x155D0) == 0x534C5553&&
			PS2_MEM_ReadWord(0x155D4) == 0x5F323036&&
			PS2_MEM_ReadWord(0x155D8) == 0x2E37373B));
			  
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_XIII_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	   	return;

	playerBase = PS2_MEM_ReadPointer(XIII_PLAYERBASE);
	aimBase = PS2_MEM_ReadPointer(playerBase + XIII_AIMBASE);

	float looksensitivity = (float)sensitivity;
	float zoom = PS2_MEM_ReadFloat(aimBase + XIII_ZOOM); 


	int16_t aimY = PS2_MEM_ReadInt16(aimBase + XIII_AIMY);
	int16_t aimX = PS2_MEM_ReadInt16(aimBase + XIII_AIMX);
	float aimYf = (float)aimY;
	float aimXf = (float)aimX;

	//update aim
	float ym = (float)(invertpitch ? ymouse : -ymouse);
	float dy = ym * looksensitivity * zoom / 85.f;
	AccumulateAddRemainder(&aimYf, &yAccumulator, ym, dy);
	float dx = (float)xmouse * looksensitivity * zoom / 85.f;
	AccumulateAddRemainder(&aimXf, &xAccumulator, (float)xmouse, dx);



	PS2_MEM_WriteInt16(aimBase + XIII_AIMY, (int16_t)aimYf);
	PS2_MEM_WriteInt16(aimBase + XIII_AIMX, (int16_t)aimXf);

	// PS2_MEM_WriteInt16(0x01FF9938, (int16_t)aimYf);
	// PS2_MEM_WriteInt16(0x01FF993C, (int16_t)aimXf);

}