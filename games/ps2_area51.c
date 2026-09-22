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

//#define A51_CAMBASE 0x5754AC
//#define A51_CAMBASE 0x5769EC
#define A51_CAMBASE 0x4F0000 // requires supplied patch file
//#define A51_CAMBASE 0x0001AB88

// offsets from camBase
// #define A51_CAMY 0x00FCA8F8
// #define A51_CAMX 0x00FCA8FC
#define A51_CAMY 0x538
#define A51_CAMX 0x53C




#define A51_FOV 0x5F0

#define A51_IS_PAUSED 0x4F03A8

static uint8_t PS2_A51_Status(void);
static void PS2_A51_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Area 51",
	PS2_A51_Status,
	PS2_A51_Inject,
	1,
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_AREA51 = &GAMEDRIVER_INTERFACE;

static uint32_t camBase = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_A51_Status(void)
{
	// SLUS_205.95
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x00093394) == 0x5F323035U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E39353BU);
}


//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_A51_Inject(void)
{
	// TODO: disable during
	//			cutscene
	// TODO: 60FPS fixes
	// 			broken scanner
	//			crash during FMV
	// TODO: clamp turret camY
	// TODO: fix slow camX on elevators/office chairs

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	
	// if (PS2_MEM_ReadUInt(A51_IS_PAUSED))
	// 	return;
	
	camBase = PS2_MEM_ReadPointer(A51_CAMBASE);

	// if (!camBase)
	// 	return;

	float looksensitivity = (float)sensitivity / 40.f;
	float scale = 600.f;
	float fov = PS2_MEM_ReadFloat(camBase + A51_FOV) / 1.04719758f;

 	float camY = PS2_MEM_ReadFloat(camBase + A51_CAMY);
	float camX = PS2_MEM_ReadFloat(camBase + A51_CAMX);

	// float camY = PS2_MEM_ReadFloat(0xFC95B8);
	// float camX = PS2_MEM_ReadFloat(0xFC95BC);

	camX -= xmouse * looksensitivity / scale * fov;
	// while (camX > TAU)
	// 	camX -= TAU;
	// while (camX < -TAU)
	// 	camX += TAU;

	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;
	camY = ClampFloat(camY, -1.518436432f, 1.518436432f);

	PS2_MEM_WriteFloat(camBase + A51_CAMY, (float)camY);
	PS2_MEM_WriteFloat(camBase + A51_CAMX, (float)camX);

	// PS2_MEM_WriteFloat(0xFC95B8, (float)camY);
	// PS2_MEM_WriteFloat(0xFC95BC, (float)camX);

}