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

#define COD2_CAM_Y 0x0039A748 // camY (static)
#define COD2_CAM_X 0x0039A74C // camX (static)
#define COD2_FOV 0x003C07E0 // 65 is default fov, used for scaling sensitivity with fov changes
#define COD2_AIM_ASSIST 0x003D8300 // 0 = aim assist off, 1 = aim assist on
#define COD2_IS_PAUSED 0x003936D4 // 0 = not paused, 1 = paused

static uint8_t PS2_COD2BRO_Status(void);
static void PS2_COD2BRO_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Call of Duty 2 - Big Red One",
	PS2_COD2BRO_Status,
	PS2_COD2BRO_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_CALLOFDUTY2BRO = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_COD2BRO_Status(void)
{
	// SLUS_212.28
	return (
		PS2_MEM_ReadWord(0x00393C78) == 0x534C5553U &&
		PS2_MEM_ReadWord(0x00393C7C) == 0x5F323132U &&
		PS2_MEM_ReadWord(0x00393C80) == 0x2E32383BU
	);
}

static void PS2_COD2BRO_Inject(void)
{

	//Disable aim assist
	PS2_MEM_WriteUInt(COD2_AIM_ASSIST, 0x0);

	//If mouse is idle
	if(xmouse == 0 && ymouse == 0) 
		return;

	//If game is paused
	if(PS2_MEM_ReadUInt(COD2_IS_PAUSED) == 0x1)
		return;

	float looksensitivity = (float)sensitivity / 45.f;
	float scale = 6.f;
	float fov = PS2_MEM_ReadFloat(COD2_FOV) / 65.f;

	float camX = PS2_MEM_ReadFloat(COD2_CAM_X);
	camX -= (float)xmouse * looksensitivity / scale * fov;
	PS2_MEM_WriteFloat(COD2_CAM_X, (float)camX);

	float camY = PS2_MEM_ReadFloat(COD2_CAM_Y);
	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;
	// game clamps internally to actual camY
	PS2_MEM_WriteFloat(COD2_CAM_Y, (float)camY);

}