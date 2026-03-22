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
#define MP2_PLAYERBASE 0x60A980 //player base pointer
#define MP2_CAMBASEX 0x15C	   //offset from playerbase
#define MP2_CAMBASEY 0x364	   //offset from playerbase
//#define MP2_ZOOMBASE 0x0066B988   //sniper zoom FOV pointer 

// States

// aim offsets from cambase
#define MP2_CAMY 0x20 //offset from camBaseY

//CAMX OFFSETS FROM camBase
#define MP2_CAMXSIN  0xA8
#define MP2_CAMXCOS  0xA0
#define MP2_CAMXNSIN  0xC0
#define MP2_CAMXCOS2  0xC8

//#define MP2_ZOOM 0x1C28 //offset from zoombase

#define MP2_ZOOM 0x74186C



static uint8_t PS2_MAXPAYNE2_Status(void);
static void PS2_MAXPAYNE2_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Max Payne 2: The Fall of Max Payne",
		PS2_MAXPAYNE2_Status,
		PS2_MAXPAYNE2_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_MAXPAYNE2 = &GAMEDRIVER_INTERFACE;

static uint32_t playerBase = 0;
static uint32_t camBaseX = 0;
static uint32_t camBaseY = 0;
static uint32_t zoomBase = 0;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_MAXPAYNE2_Status(void)
{
	// SLUS_208.14 (0x93390)
	return (PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323038&&
			PS2_MEM_ReadWord(0x93398) == 0x2E31343B);
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_MAXPAYNE2_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	   	return;

	playerBase = PS2_MEM_ReadPointer(MP2_PLAYERBASE);
	camBaseX = PS2_MEM_ReadPointer(playerBase + MP2_CAMBASEX);
	camBaseX = PS2_MEM_ReadPointer(camBaseX + 0x20);

	camBaseY = PS2_MEM_ReadPointer(playerBase + MP2_CAMBASEY);
	//zoomBase = PS2_MEM_ReadPointer(MP2_ZOOMBASE);

	float looksensitivity = (float)sensitivity;
	//float zoom = PS2_MEM_ReadFloat(zoomBase + MP2_ZOOM); 
				 //PS2_MEM_ReadFloat(0xA482C8); 
				 //1; 

	float zoom = PS2_MEM_ReadFloat(MP2_ZOOM); 


	float camY = PS2_MEM_ReadFloat(camBaseY + MP2_CAMY);
	//float camY = PS2_MEM_ReadFloat(0x0108C020);
		   
	float camXsin = PS2_MEM_ReadFloat(camBaseX + MP2_CAMXSIN);
	float camXcos = PS2_MEM_ReadFloat(camBaseX + MP2_CAMXCOS);
	//float camXsin = PS2_MEM_ReadFloat(0x013B9788);
	//float camXcos = PS2_MEM_ReadFloat(0x013B9780);
	float angle = atan(camXsin / camXcos);


	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * zoom / 15000.f;
	//camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity / 15000.f;
	camY = ClampFloat(camY, -1.553343058f, 1.553343058f);

	if(camXcos < 0)
		angle -= PI;

	angle -= (float)xmouse * looksensitivity * zoom / 15000.f;
	//angle -= (float)xmouse * looksensitivity / 15000.f;

	camXsin = sin(angle);
	camXcos = cos(angle);

	PS2_MEM_WriteFloat(camBaseY + MP2_CAMY, camY);

	PS2_MEM_WriteFloat(camBaseX + MP2_CAMXSIN, camXsin);
	PS2_MEM_WriteFloat(camBaseX + MP2_CAMXCOS, camXcos);
	PS2_MEM_WriteFloat(camBaseX + MP2_CAMXNSIN, -camXsin);
	PS2_MEM_WriteFloat(camBaseX + MP2_CAMXCOS2, camXcos);

}