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
#define GR2_CAMXBASE 0xE8EB74
#define GR2_CAMYBASE 0x72C490

//offset from camBaseY
#define GR2_CAMY 0x620

//offsets from camBaseX
#define GR2_CAMX 0x54
#define GR2_CAMX2 0x250

//not an offset
#define GR2_FOV 0xE8ED1C //Field of view

static uint8_t PS2_GR2_Status(void);
static void PS2_GR2_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Ghost Recon 2",
		PS2_GR2_Status,
		PS2_GR2_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_GHOSTRECON2 = &GAMEDRIVER_INTERFACE;

static uint32_t camBaseX = 0;
static uint32_t camBaseY = 0;
static uint32_t fovBase = 0;

static float xAccumulator = 0.f;
static float xAccumulator2 = 0.f;
static float yAccumulator = 0.f;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_GR2_Status(void)
{
	// SLUS_211.05;

	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323131&&
			PS2_MEM_ReadWord(0x93398) == 0x2E30353B)||
					
			(PS2_MEM_ReadWord(0x15B90) == 0x534C5553&&
			PS2_MEM_ReadWord(0x15B94) == 0x5F323131&&
			PS2_MEM_ReadWord(0x15B98) == 0x2E30353B)||
					
			(PS2_MEM_ReadWord(0x15B90) == 0x534C5553&&
			PS2_MEM_ReadWord(0x15B94) == 0x5F323131&&
			PS2_MEM_ReadWord(0x15B98) == 0x2E303500));		  
			  
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_GR2_Inject(void)
{

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	int16_t scale = 2;
	
	camBaseX = PS2_MEM_ReadPointer(GR2_CAMXBASE);
	camBaseY = PS2_MEM_ReadPointer(GR2_CAMYBASE);

	float looksensitivity = (float)sensitivity;
	//float fov = PS2_MEM_ReadFloat(camBaseX + GR2_FOV) / 70.f;
	float fov = PS2_MEM_ReadFloat(GR2_FOV) / 70.f;

	// while not aiming
	int16_t camY = PS2_MEM_ReadInt16(camBaseY + GR2_CAMY);
	int16_t camX = PS2_MEM_ReadInt16(camBaseX + GR2_CAMX);
	//int16_t camX2 = PS2_MEM_ReadInt16(camBaseX + GR2_CAMX2);
	
	//int16_t camY = PS2_MEM_ReadInt16(0x11BF4C0);
	//int16_t camX = PS2_MEM_ReadInt16(0x18F3024);
	float camYf = (float)camY;
	float camXf = (float)camX;
	//float camXf2 = (float)camX;

	//update cam
	float ym = (float)(invertpitch ? ymouse : -ymouse);
	float dy = ym * looksensitivity * fov / scale;
	AccumulateAddRemainder(&camYf, &yAccumulator, ym, dy);
	
	float dx = (float)xmouse * looksensitivity * fov / scale;
	AccumulateAddRemainder(&camXf, &xAccumulator, (float)xmouse, dx);

	//float dx2 = (float)xmouse * looksensitivity * fov / scale;
	//AccumulateAddRemainder(&camXf2, &xAccumulator2, (float)xmouse, dx2);
	

	//write cam
	PS2_MEM_WriteInt16(camBaseY + GR2_CAMY, (int16_t)camYf);
	PS2_MEM_WriteInt16(camBaseX + GR2_CAMX, (int16_t)camXf);
	PS2_MEM_WriteInt16(camBaseX + GR2_CAMX2, (int16_t)camXf);
	// PS2_MEM_WriteInt16(0x11BF4C0, (int16_t)camYf);
	// PS2_MEM_WriteInt16(0x18F3024, (int16_t)camXf);
	// PS2_MEM_WriteInt16(0x18F3220, (int16_t)camXf);
}