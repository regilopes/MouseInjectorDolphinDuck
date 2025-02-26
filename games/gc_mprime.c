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
#include "math.h"

#define PI 3.14159265f // 0x40490FDB

//camBasePtr
//#define MPRIME_AIMBASEPTR_X 0x437780
//#define MPRIME_AIMBASEPTR_Y 0x437818

//aim offsets from aimbase
#define MPRIME_AIMX_SIN 0x8046CA30
#define MPRIME_AIMX_COS 0x8046CA2C
#define MPRIME_AIMY 0x8046CDE4
//#define MPRIME_AIMY 0x808AFBE4 //aimY 2.0
//#define MPRIME_AIMY 0x8046CDE8 //aimY 3.0


static uint8_t GC_MPRIME_Status(void);
static void GC_MPRIME_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Metroid Prime",
	GC_MPRIME_Status,
	GC_MPRIME_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_GC_MPRIME = &GAMEDRIVER_INTERFACE;

//static uint32_t aimBaseX = 0;
//static uint32_t aimBaseY = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t GC_MPRIME_Status(void)
{
	// G4BE08
	return (MEM_ReadUInt(0x80000000) == 0x474D3845U && 
			MEM_ReadUInt(0x80000004) == 0x30310002U);

}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void GC_MPRIME_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	//float fov = MEM_ReadFloat(MPRIME_FOV);
	float looksensitivity = (float)sensitivity;
	
	float scale = 800.f;

	//aimBaseX = MEM_ReadPointer(MPRIME_AIMBASEPTR_X);
	//aimBaseY = MEM_ReadPointer(MPRIME_AIMBASEPTR_Y);

	float aimXsin = MEM_ReadFloat(MPRIME_AIMX_SIN);//while aiming
	float aimXcos = MEM_ReadFloat(MPRIME_AIMX_COS); 
	float angle = atan(aimXsin / aimXcos);

	if(aimXcos < 0)
		angle -= PI;

	angle += (float)xmouse * looksensitivity / (scale * 20);

	aimXsin = sin(angle);
	aimXcos = cos(angle);


	
	float aimY = MEM_ReadFloat(MPRIME_AIMY); 




	aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale * 25);

	//aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * 1000000.f;

	//aimY = ClampFloat(aimY, -0.90f, 0.90f);
	


	MEM_WriteFloat(MPRIME_AIMX_SIN, aimXsin);
	MEM_WriteFloat(MPRIME_AIMX_COS, aimXcos);
	MEM_WriteFloat(MPRIME_AIMY, aimY);

	//MEM_WriteFloat(MPRIME_CAMX, camX);
	//MEM_WriteFloat(MPRIME_CAMY, camY);
	
	
}