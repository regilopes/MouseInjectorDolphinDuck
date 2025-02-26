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
//#define MPRIME0_AIMBASEPTR_X 0x000000
//#define MPRIME0_AIMBASEPTR_Y 0x000000

//aim offsets from aimbase
#define MPRIME0_AIMX_SIN 0x8046B9B4
#define MPRIME0_AIMX_COS 0x8046B9B0
#define MPRIME0_AIMY 0x8046BD68
//#define MPRIME0_AIMY 0x808BB154 //aimY alternative
//#define MPRIME0_AIMY 0x808AE774 //aimY alternative deprec



static uint8_t GC_MPRIME0_Status(void);
static void GC_MPRIME0_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Metroid Prime rev0",
	GC_MPRIME0_Status,
	GC_MPRIME0_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_GC_MPRIME0 = &GAMEDRIVER_INTERFACE;

//static uint32_t aimBaseX = 0;
//static uint32_t aimBaseY = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t GC_MPRIME0_Status(void)
{
	// G4BE08
	return (MEM_ReadUInt(0x80000000) == 0x474D3845U && 
			MEM_ReadUInt(0x80000004) == 0x30310000U);

}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void GC_MPRIME0_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	//float fov = MEM_ReadFloat(MPRIME0_FOV);
	float looksensitivity = (float)sensitivity;
	
	float scale = 800.f;

	//aimBaseX = MEM_ReadPointer(MPRIME0_AIMBASEPTR_X);
	//aimBaseY = MEM_ReadPointer(MPRIME0_AIMBASEPTR_Y);

	float aimXsin = MEM_ReadFloat(MPRIME0_AIMX_SIN);//while aiming
	float aimXcos = MEM_ReadFloat(MPRIME0_AIMX_COS); 
	float angle = atan(aimXsin / aimXcos);

	if(aimXcos < 0)
		angle -= PI;

	angle += (float)xmouse * looksensitivity / (scale * 20.0f);

	aimXsin = sin(angle);
	aimXcos = cos(angle);

	
	float aimY = MEM_ReadFloat(MPRIME0_AIMY); 
	

	aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale * 25.0f);

		
	MEM_WriteFloat(MPRIME0_AIMX_SIN, aimXsin);
	MEM_WriteFloat(MPRIME0_AIMX_COS, aimXcos);
	
	
	aimY = ClampFloat(aimY, -1.22f, 1.22f);
	//aimY = ClampFloat(aimY, -0.94f, 0.94f);
	MEM_WriteFloat(MPRIME0_AIMY, aimY);
	

	//MEM_WriteFloat(MPRIME0_CAMX, camX);
	//MEM_WriteFloat(MPRIME0_CAMY, camY);
	
}