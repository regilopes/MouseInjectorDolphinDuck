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
#define TAU 6.2831853f // 0x40C90FDB


// pointers
#define TP_CAMBASE 0x4B0EF8

// States
#define TP_FINEAIM 0x48A5FD

//OFFSETS FROM CAMBASE
#define TP_CAMY 0x28C 
#define TP_CAMX  0x290
#define TP_AIMY 0x320

#define TP_AIMXSIN  0x31C
#define TP_AIMXCOS  0x324
#define TP_AIMXNSIN  0x30C
#define TP_AIMXCOS2  0x304


#define TP_BODYX  0x77C

#define TP_FOV 0x53A4F4



static uint8_t PS2_PUNISHER_Status(void);
static void PS2_PUNISHER_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"The Punisher",
		PS2_PUNISHER_Status,
		PS2_PUNISHER_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_PUNISHER = &GAMEDRIVER_INTERFACE;

static uint32_t playerBase = 0;
static uint32_t camBase = 0;
static uint8_t aimState = 0;



//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_PUNISHER_Status(void)
{
	// SLUS_208.64 (0x93390)
	return (PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323038&&
			PS2_MEM_ReadWord(0x93398) == 0x2E36343B);
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_PUNISHER_Inject(void)
{
	// if(xmouse == 0 && ymouse == 0) // if mouse is idle
	//    	return;

	camBase = PS2_MEM_ReadPointer(TP_CAMBASE);
	aimState = PS2_MEM_ReadUInt8(TP_FINEAIM);


	float looksensitivity = (float)sensitivity;
	float scale = 1500.f;

	float camY = PS2_MEM_ReadFloat(camBase + TP_CAMY);
	float camX = PS2_MEM_ReadFloat(camBase + TP_CAMX);

	float fov = pow(PS2_MEM_ReadFloat(TP_FOV)/60.f, 3.f);

	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity / scale * fov;
	camY *= 0.9f; // Damping factor to smooth the rotation
	
	camX += (float)xmouse * looksensitivity / scale * fov;
	camX *= 0.9f; // Damping factor to smooth the rotation


	PS2_MEM_WriteFloat(camBase + TP_CAMX, camX);
	PS2_MEM_WriteFloat(camBase + TP_CAMY, camY);
}

