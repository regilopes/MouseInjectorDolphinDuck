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

#define WAWFF_CAMY 0x1EA9CC0
#define WAWFF_CAMX 0x1EA9CC4
#define WAWFF_ZOOM 0x4DC048

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

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_WAWFF_Status(void)
{
	// SLUS_217.46
	return (PS2_MEM_ReadWord(0x93390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x93394) == 0x5F323137U &&
			PS2_MEM_ReadWord(0x93398) == 0x2E34363BU);
}

static void PS2_WAWFF_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	float looksensitivity = (float)sensitivity / 40.f;
	float scale = 400.f;
	float zoom = 1.56968534f / PS2_MEM_ReadFloat(WAWFF_ZOOM);

	float camX = PS2_MEM_ReadFloat(WAWFF_CAMX);
	camX += (float)xmouse * looksensitivity / scale * zoom;
	PS2_MEM_WriteFloat(WAWFF_CAMX, (float)camX);

	float camY = PS2_MEM_ReadFloat(WAWFF_CAMY);
	camY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * zoom;
	camY = ClampFloat(camY, -1.299999952f, 1.299999952f);
	PS2_MEM_WriteFloat(WAWFF_CAMY, (float)camY);

}