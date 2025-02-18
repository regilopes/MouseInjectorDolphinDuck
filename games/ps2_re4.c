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

#define TAU 6.2831853f // 0x40C90FDB
//camBasePtr
#define RE4_AIMBASEPTR_X 0x437780
#define RE4_AIMBASEPTR_Y 0x437818

//aim offsets from aimbase
#define RE4_AIMX 0xD4
#define RE4_AIMY 0x28
//#define RE4_AIMY 0x3D7564  //aimY 2.0

//cam coords
//#define RE4_CAMX 0x3CB008
//#define RE4_CAMY 0x3CB004

#define RE4_CAMX 0x3CA7B8
#define RE4_CAMY 0x3CA7B4

// #define RE4_fFOV1 0x102DCC
// #define RE4_fFOV2 0x102DDC
#define RE4_FOV 0x3CB014

static uint8_t PS2_RE4_Status(void);
static void PS2_RE4_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Resident Evil 4",
	PS2_RE4_Status,
	PS2_RE4_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_RE4 = &GAMEDRIVER_INTERFACE;

static uint32_t aimBaseX = 0;
static uint32_t aimBaseY = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_RE4_Status(void)
{
	//SLUS_211.34 (0x93390) (0x90708)
	// 53 4C 55 53 | 5F 32 31 31 | 2E 33 34 3B   
	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553 &&
			PS2_MEM_ReadWord(0x93394) == 0x5F323131 &&
			PS2_MEM_ReadWord(0x93398) == 0x2E33343B) ||

			(PS2_MEM_ReadWord(0x90708) == 0x534C5553 &&
			PS2_MEM_ReadWord(0x9070C) == 0x5F323131 &&
			PS2_MEM_ReadWord(0x90710) == 0x2E33343B));
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_RE4_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	float fov = PS2_MEM_ReadFloat(RE4_FOV);
	float looksensitivity = (float)sensitivity / fov;
	
	float scale = 800.f;

	aimBaseX = PS2_MEM_ReadPointer(RE4_AIMBASEPTR_X);
	aimBaseY = PS2_MEM_ReadPointer(RE4_AIMBASEPTR_Y);

	float aimX = PS2_MEM_ReadFloat(0x188061C); //while aiming
	float aimY = PS2_MEM_ReadFloat(aimBaseY + RE4_AIMY); 
	//float aimY = PS2_MEM_ReadFloat(RE4_AIMY); //aimY 2.0

	float camX = PS2_MEM_ReadFloat(RE4_CAMX); //while not aiming
	float camY = PS2_MEM_ReadFloat(RE4_CAMY);


	aimX += (float)-xmouse * looksensitivity / scale;
	

	aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale/1.4f);
	//aimY += (float)(!invertpitch ? -ymouse : ymouse) / 30.f;      //aimY 2.0
	aimY = ClampFloat(aimY, -1.570796371f, 1.570796371f);
	//aimY = ClampFloat(aimY, -1.f, 1.f);  //aimY2.0

	
	camX += (float)-xmouse * looksensitivity / scale * 2;

	// while (camX >= TAU)
	//  		camX -= TAU;
	// while (camX < 0)
	//  		camX += TAU;
	

	camY += (float)-ymouse / 600.f;
	camY = ClampFloat(camY, -2.0f, 2.f);

	PS2_MEM_WriteFloat(0x188061C, aimX);
	PS2_MEM_WriteFloat(aimBaseY + RE4_AIMY, aimY);
	//PS2_MEM_WriteFloat(RE4_AIMY, aimY);  //aimY 2.0

	PS2_MEM_WriteFloat(RE4_CAMX, camX);
	PS2_MEM_WriteFloat(RE4_CAMY, camY);
	
	
}