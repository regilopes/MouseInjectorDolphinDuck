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

//TODO:
//fix glitch when entering a new map (gameState loading)

//cam and aim pointer (viewBase)
//#define MGS3_VIEWBASE 0x0020CF18
#define MGS3_CAMBASE 0x0020CEE0
#define MGS3_AIMBASE 0x0024EEF8
// #define MGS3_AIMBASE 0x0020CF18


//States
#define MGS3_VIEWSTATE 0x001D7463 //aiming or not aiming
#define MGS3_GAMESTATE 0x0024EC8C //loading new area or not

//cam offsets from viewbase
#define MGS3_CAMY -0xC0
#define MGS3_CAMX -0xC8

//aim offsets from viewbase
#define MGS3_AIMY 0xC670
#define MGS3_AIMX 0xC672
// #define MGS3_AIMY 0x2E0
// #define MGS3_AIMX 0x2E2

// #define MGS3_FOV -0x18D64
//#define MGS3_FOV -0x14E84
#define MGS3_FOV -0x8AF4


static uint8_t PS2_MGS3_Status(void);
static void PS2_MGS3_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Metal Gear Solid 3: Subsistence",
	PS2_MGS3_Status,
	PS2_MGS3_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_MGS3 = &GAMEDRIVER_INTERFACE;

static uint32_t camBase = 0;
static uint32_t aimBase = 0;
static uint8_t viewState = 0;
static uint16_t gameState = 0;
static float xAccumulator = 0.f;
static float yAccumulator = 0.f;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_MGS3_Status(void)
{
	//SLUS_213.59 (0x93390) (0x90708)
	// 53 4C 55 53 | 5F 32 31 33 | 2E 35 39 3B
	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553 &&
			 PS2_MEM_ReadWord(0x93394) == 0x5F323133 &&
			 PS2_MEM_ReadWord(0x93398) == 0x2E35393B)||

			(PS2_MEM_ReadWord(0x90708) == 0x534C5553 &&
			 PS2_MEM_ReadWord(0x9070C) == 0x5F323133 &&
			 PS2_MEM_ReadWord(0x90710) == 0x2E35393B));
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_MGS3_Inject(void)
{
	
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	 	return;
	
	gameState = PS2_MEM_ReadUInt16(MGS3_GAMESTATE);
	if(gameState != 0x0080) //if game is loading new area
		return;

	camBase = PS2_MEM_ReadPointer(MGS3_CAMBASE);
	aimBase = PS2_MEM_ReadPointer(MGS3_AIMBASE);
	viewState = PS2_MEM_ReadUInt8(MGS3_VIEWSTATE);
	

	float fov = PS2_MEM_ReadFloat(aimBase + MGS3_FOV);
	float looksensitivity = (float)sensitivity;

	//while not aiming
	float camY = PS2_MEM_ReadFloat(camBase + MGS3_CAMY);
	uint16_t camX = PS2_MEM_ReadUInt(camBase + MGS3_CAMX);

	//while aiming
	uint16_t aimY = PS2_MEM_ReadUInt16(aimBase + MGS3_AIMY);
	uint16_t aimX = PS2_MEM_ReadUInt16(aimBase + MGS3_AIMX);
	float aimYf = (float)aimY;
	float aimXf = (float)aimX;
	
	

	//update values from mouse movement
	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity / 50000.f;
	camY = ClampFloat(camY, 0.f, 1.f);

	camX += (uint16_t)-xmouse * looksensitivity / 2;
	


	float ym = (float)(invertpitch ? -ymouse : ymouse);
	float dy = ym * looksensitivity / fov / 2.f;
	AccumulateAddRemainder(&aimYf, &yAccumulator, ym, dy);

	float dx = (float)-xmouse * looksensitivity / fov / 2.f;
	AccumulateAddRemainder(&aimXf, &xAccumulator, (float)xmouse, dx);

	// aimY += (uint16_t)(!invertpitch ? ymouse : -ymouse) * looksensitivity / fov / 2;
	// aimX += (uint16_t)-xmouse * looksensitivity / fov / 2;
	//aimX = ClampInt(aimX, 0, 65536);
	
	if (viewState == 0x00){
		PS2_MEM_WriteFloat(camBase + MGS3_CAMY, camY);
		PS2_MEM_WriteUInt16(camBase + MGS3_CAMX, camX);
	}
	
	if (viewState == 0x04 ||// first person
		viewState == 0x14){ //binoculars/scope
		PS2_MEM_WriteUInt16(aimBase + MGS3_AIMY, (uint16_t)aimYf);
		PS2_MEM_WriteUInt16(aimBase + MGS3_AIMX, (uint16_t)aimXf);
	}
}