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
#include <stdio.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define QUAKE_ROTX 0x80117F12
#define QUAKE_ROTY 0x80117F14
#define QUAKE_ZOOM 0x80117F44


#define QUAKE_CAMX 0x80163840
#define QUAKE_CAMY 0x8016383c

static uint8_t N64_QUAKE_Status(void);
static void N64_QUAKE_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Quake",
	N64_QUAKE_Status,
	N64_QUAKE_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_N64_QUAKE = &GAMEDRIVER_INTERFACE;

static float xAccumulator = 0.f;
static float yAccumulator = 0.f;

static uint8_t N64_QUAKE_Status(void)
{
	return (N64_MEM_ReadUInt(0x80000000) == 0x3C1A8006 && N64_MEM_ReadUInt(0x80000004) == 0x275AF020); // unique header in RDRAM
	//return (N64_MEM_ReadUInt(0x80000000) == 0x06801A3C && N64_MEM_ReadUInt(0x80000004) == 0x20F05A27);
}

static void N64_QUAKE_Inject(void)
{

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	// const float looksensitivity = (float)sensitivity;
	// const float scale = 1.f;
	// const float zoom = 1.f;

	// int16_t rotX = N64_MEM_ReadInt16(QUAKE_CAMX);
	// int16_t rotY = N64_MEM_ReadInt16(QUAKE_CAMY);

	// printf("Quake: rotX: %d, rotY: %d\n", rotX, rotY);

	// float rotXF = (float)rotX;
	// float rotYF = (float)rotY;
	
	// float dx = (float)xmouse * looksensitivity / scale * zoom;
	// AccumulateAddRemainder(&rotXF, &xAccumulator, xmouse, dx);
	// // cursorXF = ClampFloat(cursorXF, 8.f, 312.f);

	// float ym = (float)(invertpitch ? -ymouse : ymouse);
	// float dy = ym * looksensitivity / scale * zoom;
	// AccumulateAddRemainder(&rotYF, &yAccumulator, ym, dy);
	// //rotYF = ClampFloat(rotYF, -301.f, 301.f);

	// // rotXF -= (float)xmouse * looksensitivity * scale * zoom;
	// // rotYF -= (float)ymouse * looksensitivity * scale * zoom;

	// //N64_MEM_WriteInt16(QUAKE_ROTX, (int16_t)rotXF);
	// //N64_MEM_WriteInt16(QUAKE_ROTY, (int16_t)rotYF);
	
	// N64_MEM_WriteInt16(QUAKE_CAMX, (int16_t)rotXF);
	// N64_MEM_WriteInt16(QUAKE_CAMY, (int16_t)rotYF);


	float camX = N64_MEM_ReadFloat(QUAKE_CAMX);
	float camY = N64_MEM_ReadFloat(QUAKE_CAMY);
	
	camX -= (float)(xmouse / 30.f); // normal calculation method for X
	camY += (float)(!invertpitch ? ymouse : -ymouse) / 30.f; // normal calculation method for Y

	N64_MEM_WriteFloat(QUAKE_CAMX, camX);
	N64_MEM_WriteFloat(QUAKE_CAMY, camY);

}