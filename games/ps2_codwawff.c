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

#define WAWFF_PLAYER_CONTROL 0x004D8F38 //0 = player has control, 1 = player has no control (cutscene, paused, etc).
#define WAWFF_ZOOM 0x4DC048 //Weapon zoom

#define WAWFF_CURRENT_MISSION 0x483a40 //In Mission 07 "The Race to Bastogne" you play only in a Tank so we can use this Adress for Tank Values.

#define WAWFF_CAMBASE_PTR 0x005980C0 
//offsets from camBase
#define WAWFF_CAM_X 0x14 
#define WAWFF_CAM_Y 0x10
//camX Value 1: 0x1EA9CC4 | Documentation for two different camX values to get the Pointer for camBase.
//camY Value 1: 0x1EA9CC0 | Documentation for two different camX values to get the Pointer for camBase.

//tankX 01D7D104 tankY 01D7D100

static uint8_t PS2_WAWFF_Status(void);
static void PS2_WAWFF_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Call of Duty: World at War - Final Fronts",
	PS2_WAWFF_Status,
	PS2_WAWFF_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_CALLOFDUTYWORLDATWAR = &GAMEDRIVER_INTERFACE;

static uint32_t camBase = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_WAWFF_Status(void)
{
	// SLUS_217.46
	return (
		PS2_MEM_ReadWord(0x00093390) == 0x534C5553U &&
		PS2_MEM_ReadWord(0x00093394) == 0x5F323137U &&
		PS2_MEM_ReadWord(0x00093398) == 0x2E34363BU
	);
}

static void PS2_WAWFF_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	if (PS2_MEM_ReadUInt(WAWFF_PLAYER_CONTROL) != 0x0) //if player doesn't have control, don't inject. This covers paused, cutscenes, menu and settings states.
		return;

	camBase = PS2_MEM_ReadPointer(WAWFF_CAMBASE_PTR);

	float looksensitivity = (float)sensitivity / 40.f;
	float scale = 400.f;
	float zoom = 1.56968534f / PS2_MEM_ReadFloat(WAWFF_ZOOM);

	float camX = PS2_MEM_ReadFloat(camBase + WAWFF_CAM_X);
	float camY = PS2_MEM_ReadFloat(camBase + WAWFF_CAM_Y);

	if (PS2_MEM_ReadUInt(WAWFF_CURRENT_MISSION) == 0x7)
	{

		camX += (float)xmouse * looksensitivity / scale * zoom;
			while (camX > PI)
				camX -= TAU;
			while (camX < -PI)
				camX += TAU;
	
		camY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * zoom;
		camY = ClampFloat(camY, -0.122173056f, 0.1745329499f);

		PS2_MEM_WriteFloat(camBase + WAWFF_CAM_X, (float)camX);
		PS2_MEM_WriteFloat(camBase + WAWFF_CAM_Y, (float)camY);

	}
	else
	{

	float looksensitivity = (float)sensitivity / 40.f;
	float scale = 400.f;
	float zoom = 1.56968534f / PS2_MEM_ReadFloat(WAWFF_ZOOM);

	float camX = PS2_MEM_ReadFloat(camBase + WAWFF_CAM_X);
	float camY = PS2_MEM_ReadFloat(camBase + WAWFF_CAM_Y);

		camX += (float)xmouse * looksensitivity / scale * zoom;
		while (camX > PI)
				camX -= TAU;
			while (camX < -PI)
				camX += TAU;
		PS2_MEM_WriteFloat(camBase + WAWFF_CAM_X, (float)camX);
	
		camY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * zoom;
		camY = ClampFloat(camY, -1.299999952f, 1.299999952f);
		PS2_MEM_WriteFloat(camBase + WAWFF_CAM_Y, (float)camY);
		}
}