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
#define H2_GLOBALBASE 0x4da038	 //pointer to many things
#define H2_CAMBASEX 0x01fe179c	 //camX
#define H2_SCOPEBASE 0x01ff73f0	 //camX


// States
#define H2_PAUSESTATE 0x005108C8 // paused or not
#define H2_CUTSCENESTATE 0x005080F0 // in cutscene or not 
#define H2_SCOPESTATE 0x124 //looking into binocular/scope or not (playerBase offset)


// cam offsets from playerBase
#define H2_CAMY 0xD54    //third person
#define H2_FPY 0x13E4    //first person
#define H2_SCOPEY 0x1580 //scope/binoculars

// cam offsets from camBaseX
#define H2_CAMXSIN 0x0
#define H2_CAMXCOS 0x8

#define H2_CAMXSINB 0x20 //needed to fix body animation
#define H2_CAMXCOSB 0x18 //needed to fix body animation


#define H2_ZOOM 0x568 //offset from globalBase

static uint8_t PS2_H2_Status(void);
static void PS2_H2_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Hitman 2: Silent Assassin",
		PS2_H2_Status,
		PS2_H2_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_HITMAN2 = &GAMEDRIVER_INTERFACE;

static uint32_t globalBase = 0;
static uint32_t camBaseX = 0;
static uint32_t playerBase = 0;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_H2_Status(void)
{
	// STARTUP.ELF; (0x93390)
	// 53 54 41 52 | 54 55 50 2E | 45 4C 46 3B
	return (PS2_MEM_ReadWord(0x93390) == 0x53544152&&
			PS2_MEM_ReadWord(0x93394) == 0x5455502E&&
			PS2_MEM_ReadWord(0x93398) == 0x454C463B);
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_H2_Inject(void)
{
	// if(xmouse == 0 && ymouse == 0) // if mouse is idle
	//    	return;

	if(PS2_MEM_ReadUInt8(H2_PAUSESTATE) == 0x01) //paused or inventory
		return;

	if(PS2_MEM_ReadUInt8(H2_CUTSCENESTATE) == 0x01) //in cutscene
		return;

	//60FPS
	if(PS2_MEM_ReadUInt8(0x508090) == 0x02)
		PS2_MEM_WriteUInt8(0x508090, 0x01);

	
	globalBase = PS2_MEM_ReadPointer(H2_GLOBALBASE);

	playerBase = PS2_MEM_ReadPointer(globalBase + 0x20);
	playerBase = PS2_MEM_ReadPointer(playerBase + 0x54);
	playerBase = PS2_MEM_ReadPointer(playerBase + 0x654);

	camBaseX = PS2_MEM_ReadPointer(H2_CAMBASEX);
	

	float zoom = PS2_MEM_ReadFloat(globalBase + H2_ZOOM);
	float looksensitivity = (float)sensitivity;


	float camY = PS2_MEM_ReadFloat(playerBase + H2_CAMY);

	float scopeY = PS2_MEM_ReadFloat(playerBase + H2_SCOPEY);

	float fpY = PS2_MEM_ReadFloat(playerBase + H2_FPY);


	float camXsin = PS2_MEM_ReadFloat(camBaseX + H2_CAMXSIN);
	float camXcos = PS2_MEM_ReadFloat(camBaseX + H2_CAMXCOS);
	float angle = atan(camXsin / camXcos);

	float camXsinB = PS2_MEM_ReadFloat(camBaseX + H2_CAMXSINB);
	float camXcosB = PS2_MEM_ReadFloat(camBaseX + H2_CAMXCOSB);
	float angleB = atan(camXsinB / camXcosB);

	//update cam
	scopeY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 300.f;
	scopeY = ClampFloat(scopeY, -88.f, 88.f);

	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * zoom / 15000.f;
	camY = ClampFloat(camY, -1.04719758f, 1.431169987f);

	fpY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 15000.f;
	fpY = ClampFloat(fpY, -1.22173059f, 1.22173059f);

	
	if(camXcos < 0)
		angle -= PI;

	angle += (float)xmouse * looksensitivity * zoom / 15000.f;

	camXsin = sin(angle);
	camXcos = cos(angle);

	if(PS2_MEM_ReadUInt8(playerBase + H2_SCOPESTATE) == 0x0)
		PS2_MEM_WriteFloat(globalBase + H2_ZOOM, 1.17f);

	PS2_MEM_WriteFloat(playerBase + H2_CAMY, camY);
	
	PS2_MEM_WriteFloat(playerBase + H2_SCOPEY, scopeY);

	PS2_MEM_WriteFloat(playerBase + H2_FPY, fpY);

	PS2_MEM_WriteFloat(camBaseX + H2_CAMXSIN, camXsin);
	PS2_MEM_WriteFloat(camBaseX + H2_CAMXCOS, camXcos);

	//fixing body rotation animation
	PS2_MEM_WriteFloat(camBaseX + H2_CAMXSINB, -camXsin);
	PS2_MEM_WriteFloat(camBaseX + H2_CAMXCOSB, camXcos);
}