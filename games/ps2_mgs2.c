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

// aiming camera pointer
#define MGS2_AIMBASE 0x001D08B8	   //aim base pointer
#define MGS2_FOVBASE 0x00180D10    //fov base pointer

// States
#define MGS2_LOADINGSTATE 0x00192E64 // loading new area or not
#define MGS2_CUTSCNSTATE 0x0	  // is FP on cutscene or not (offset from cutscnebase)
#define MGS2_WEPSTATE 0x0	  // wepon ready, reloading, empty... (offset from aimbase)

// cam offsets from cambase
//#define MGS2_CAMY -0x100
//#define MGS2_CAMX -0x108

// aim offsets from viewbase
#define MGS2_AIMY 0x001D0F20 // not an offset
#define MGS2_AIMX 0x22 
#define MGS2_AIMY2 0xAB0
#define MGS2_AIMX2 0x2A


#define MGS2_FOV 0x7A0 //offset from fovbase

static uint8_t PS2_MGS2_Status(void);
static void PS2_MGS2_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Metal Gear Solid 2: Substance",
		PS2_MGS2_Status,
		PS2_MGS2_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_MGS2 = &GAMEDRIVER_INTERFACE;

static uint32_t fovBase = 0;
static uint32_t aimBase = 0;
static uint8_t fpcamState = 0;
static float xAccumulator = 0.f;
static float yAccumulator = 0.f;
static float xAccumulator2 = 0.f;
static float yAccumulator2 = 0.f;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_MGS2_Status(void)
{
	// SLUS_205.54 (0x93390)
	//  53 4C 55 53 | 5F 32 30 35 | 2E 35 34 3B
	return (PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323035&&
			PS2_MEM_ReadWord(0x93398) == 0x2E35343B);
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_MGS2_Inject(void)
{
	// if(xmouse == 0 && ymouse == 0) // if mouse is idle
	//    	return;

	if (PS2_MEM_ReadUInt8(0x00193278) == 0x0 && //FP mode OFF
	    PS2_MEM_ReadUInt8(MGS2_LOADINGSTATE) == 0x7F)  //playing state (not loading)
			PS2_MEM_WriteUInt8(0x00193278, 0x1); //FP mode ON
	
	if (PS2_MEM_ReadUInt8(MGS2_LOADINGSTATE) == 0x80) //loading
		PS2_MEM_WriteUInt8(0x00193278, 0x0);
	

	// gameState = PS2_MEM_ReadUInt8(MGS2_GAMESTATE);
	// if (gameState != 0x01 && gameState != 0x11) // if isn't 'ingame' and 'cutscene'
	// 	return;

	aimBase = PS2_MEM_ReadPointer(MGS2_AIMBASE);
	fovBase = PS2_MEM_ReadPointer(MGS2_FOVBASE);

	// float fov = PS2_MEM_ReadFloat(aimBase + MGS2_FOV);
	float fov = PS2_MEM_ReadFloat(fovBase + MGS2_FOV);
	float looksensitivity = (float)sensitivity;


	//// while not aiming
	//float camY = PS2_MEM_ReadFloat(camBase + MGS2_CAMY);
	//int16_t camX = PS2_MEM_ReadInt16(camBase + MGS2_CAMX);

	// while aiming
	int16_t aimY = PS2_MEM_ReadInt16(MGS2_AIMY);
	int16_t aimX = PS2_MEM_ReadInt16(aimBase + MGS2_AIMX);
	float aimYf = (float)aimY;
	float aimXf = (float)aimX;

	int16_t aimY2 = PS2_MEM_ReadInt16(aimBase + MGS2_AIMY2);
	int16_t aimX2 = PS2_MEM_ReadInt16(aimBase + MGS2_AIMX2);
	float aimYf2 = (float)aimY2;
	float aimXf2 = (float)aimX2;

	//update aim
	float ym = (float)(invertpitch ? -ymouse : ymouse);
	float dy = ym * looksensitivity / fov / 40.f;
	AccumulateAddRemainder(&aimYf, &yAccumulator, ym, dy);
	float dx = (float)-xmouse * looksensitivity / fov / 30.f;
	AccumulateAddRemainder(&aimXf, &xAccumulator, (float)xmouse, dx);


	//update aim
	// float ym2 = (float)(invertpitch ? -ymouse : ymouse);
	// float dy2 = ym2 * looksensitivity / fov / 25.f;
	// AccumulateAddRemainder(&aimYf2, &yAccumulator2, ym2, dy2);
	float dx2 = (float)-xmouse * looksensitivity / fov / 6.f;
	AccumulateAddRemainder(&aimXf2, &xAccumulator2, (float)xmouse, dx2);



	// //update cam
	// aimY += (int16_t)(!invertpitch ? ymouse : -ymouse) / 2;
	// aimX += (int16_t)-xmouse / 2;
	// aimY2 += (int16_t)(!invertpitch ? ymouse : -ymouse) / 2;
	// aimX2 += (int16_t)-xmouse / 2;
 	

	PS2_MEM_WriteInt16(MGS2_AIMY, (int16_t)aimYf);
	//PS2_MEM_WriteInt16(aimBase + MGS2_AIMX, (int16_t)aimXf);
	//PS2_MEM_WriteInt16(aimBase + MGS2_AIMY2, (int16_t)aimYf2);
	PS2_MEM_WriteInt16(aimBase + MGS2_AIMX2, (int16_t)aimXf2);

	// PS2_MEM_WriteInt16(MGS2_AIMY, (int16_t)aimY);
	// PS2_MEM_WriteInt16(aimBase + MGS2_AIMX, (int16_t)aimX);
	// PS2_MEM_WriteInt16(aimBase + MGS2_AIMY2, (int16_t)aimY2);
	// PS2_MEM_WriteInt16(aimBase + MGS2_AIMX2, (int16_t)aimX2);
}