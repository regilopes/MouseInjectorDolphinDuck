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

#define AMSH_ROT_BASE_PTR 0x46BE44
// offsets from rotBase
#define AMSH_ROTX_SIN 0xA0
#define AMSH_ROTX_COS 0xA4
#define AMSH_CURSORX 0x2FCC
#define AMSH_CURSORY 0x2FD0

#define AMSH_CAMY 0x15F80F4
#define AMSH_AIMY 0x69099C
//#define AMSH_CAMY_DEST 0x4DD464
//#define AMSH_CAMX 0x4DD460
// #define AMSH_CAMX_DEST 0x4DD470
#define AMSH_BODYX 0x15F81C4

#define AMSH_FOV 0x690A58

#define AMSH_IS_BUSY 0xC9DE10

// #define AMSH_CURSORX 0xD2381C
// #define AMSH_CURSORY 0xD23820

static uint8_t PS2_AMSH_Status(void);
static void PS2_AMSH_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Army Men: Sarge's Heroes",
	PS2_AMSH_Status,
	PS2_AMSH_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_ARMYMENSARGESHEROES = &GAMEDRIVER_INTERFACE;

static uint32_t rotBase = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_AMSH_Status(void)
{
	// SLUS_201.32
	return (PS2_MEM_ReadWord(0x93390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x93394) == 0x5F323031U &&
			PS2_MEM_ReadWord(0x93398) == 0x2E33323BU);
}

static void PS2_AMSH_Inject(void)
{
	// TODO: disable during
	//			level entry
	//			level end
	//			pause
	//			menu
	// TODO: fix rotBase broken after getting in jeep

	// lock cursor to center screen right above head
	// PS2_MEM_WriteFloat(rotBase + AMSH_CURSORX, 0.f);
	// PS2_MEM_WriteFloat(rotBase + AMSH_CURSORY, 35.f);
	// PS2_MEM_WriteFloat(AMSH_CURSORX, 0.f);
	// PS2_MEM_WriteFloat(AMSH_CURSORY, 35.f);

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	
	// if (PS2_MEM_ReadUInt(AMSH_IS_BUSY) != 0x1)
	// 	return;

	// rotBase = PS2_MEM_ReadPointer(AMSH_ROT_BASE_PTR);
	// if (!rotBase)
	// 	return;
	
	float looksensitivity = (float)sensitivity / 40.f;
	float scale = 250.f;
	float fov = PS2_MEM_ReadFloat(AMSH_FOV) / 55.0f;
	
	int16_t bodyX = PS2_MEM_ReadInt16(AMSH_BODYX);
	bodyX += xmouse * looksensitivity * 30 * fov;


	PS2_MEM_WriteInt16(AMSH_BODYX, bodyX);
	
	// float rotXSin = PS2_MEM_ReadFloat(rotBase + AMSH_ROTX_SIN);
	// float rotXCos = PS2_MEM_ReadFloat(rotBase + AMSH_ROTX_COS);
	//float angle = atan(rotXSin / rotXCos);
	// if (rotXCos < 0)
	// 	angle += TAU / 2.f;
	
	// rotXSin = sin(angle);
	// rotXCos = cos(angle);

	//PS2_MEM_WriteFloat(rotBase + AMSH_ROTX_SIN, rotXSin);
	//PS2_MEM_WriteFloat(rotBase + AMSH_ROTX_COS, rotXCos);
	//PS2_MEM_WriteFloat(AMSH_CAMX, -angle * (180.f / (TAU / 2.f)));
	//PS2_MEM_WriteFloat(AMSH_CAMX_DEST, -angle * (180.f / (TAU / 2.f)));

	
	float camY = PS2_MEM_ReadFloat(AMSH_CAMY);
	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * 60 * fov;
	camY = ClampFloat(camY, -45.f, 45.f);
	PS2_MEM_WriteFloat(AMSH_CAMY, (float)camY);
	
	
	float aimY = PS2_MEM_ReadFloat(AMSH_AIMY);
	aimY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * 60 * fov;
	//aimY = ClampFloat(aimY, -85.f, 85.f);
	PS2_MEM_WriteFloat(AMSH_AIMY, (float)aimY);
	
	//PS2_MEM_WriteFloat(AMSH_CAMY_DEST, (float)camY);

}