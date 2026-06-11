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

#define RF1_AUTO_AIM 0x00C1E5DC //Won't be used because in this game AUTOAIM is just snapping the crosshair to the enemy and not messing with cam angles.
#define RF1_PLAYER_CONTROL 0x002AC0D8 //0 = player has control, 1 = player has no control (cutscene, paused, etc).

#define RF1_CAMBASEPTR 0x002ABF40 // pointer to camBase, which is used in various places for camera related values, including camX and camY. camBase is allocated on the heap and can change between runs, so we need to read the pointer every frame.
//offsets from camBase
#define RF1_CAM_X 0x78C // camX is used for horizontal aiming, it wraps around at 360 degrees (2*PI or TAU).
#define RF1_CAM_Y 0x7A0 //camY is used for vertical aiming, it is clamped between -90 and 90 degrees (-PI/2 and PI/2).

#define RF1_TURRET_X 0x78C // camX is used for horizontal aiming, it wraps around at 360 degrees (2*PI or TAU).
#define RF1_TURRET_Y 0x7A0 //camY is used for vertical aiming, it is clamped between -90 and 90 degrees (-PI/2 and PI/2).

#define RF1_TURRET_FLAG 0x01FFD238 //0x01EE23C0
#define RF1_TURRET_BASEPTR 0x0083E0D8

static uint8_t PS2_RF1_Status(void);
static void PS2_RF1_Inject(void);

//turret y 7a0

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Red Faction",
	PS2_RF1_Status,
	PS2_RF1_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_REDFACTION = &GAMEDRIVER_INTERFACE;

static uint32_t camBase = 0;
static uint32_t turretBase = 0;

static uint8_t PS2_RF1_Status(void)
{
	// SLUS_200.73
	return (
		PS2_MEM_ReadWord(0x00093390) == 0x534C5553U &&
		PS2_MEM_ReadWord(0x00093394) == 0x5F323030U &&
		PS2_MEM_ReadWord(0x00093398) == 0x2E37333BU
	);
}

static void PS2_RF1_Inject(void)
{
	if (xmouse == 0 && ymouse == 0)
		return;

	if (PS2_MEM_ReadUInt(RF1_PLAYER_CONTROL) != 0x0) //if player doesn't have control, don't inject. This covers paused, cutscenes, menu and settings states.
		return;

	camBase = PS2_MEM_ReadPointer(RF1_CAMBASEPTR);
	//turretBase = PS2_MEM_ReadPointer(RF1_TURRET_BASEPTR);

	float looksensitivity = (float)sensitivity;
	float scale = 15000.f;

	//For Turret I didn't find a good flag. Will be later fixed
	/*if (PS2_MEM_ReadUInt(RF1_TURRET_FLAG) != 0x0)
	{
		float turretX = PS2_MEM_ReadFloat(turretBase + RF1_TURRET_X);
		float turretY = PS2_MEM_ReadFloat(turretBase + RF1_TURRET_Y);

		turretX += (float)xmouse * looksensitivity / scale;
		while (turretX > TAU)
			turretX -= TAU;
		while (turretX < -TAU)
			turretX += TAU;

		turretY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;
		turretY = ClampFloat(turretY, -0.4363323152f, 0.4363323152f);

		PS2_MEM_WriteFloat(turretBase + RF1_TURRET_X, turretX);
		PS2_MEM_WriteFloat(turretBase + RF1_TURRET_Y, turretY);

	}
	else
	{*/

	float camX = PS2_MEM_ReadFloat(camBase + RF1_CAM_X);
	float camY = PS2_MEM_ReadFloat(camBase + RF1_CAM_Y);

	camX += (float)xmouse * looksensitivity / scale;
		while (camX > TAU)
			camX -= TAU;
		while (camX < -TAU)
			camX += TAU;

		camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;
		camY = ClampFloat(camY, -1.570796371f, 1.570796371f);

	PS2_MEM_WriteFloat(camBase + RF1_CAM_X, camX);
	PS2_MEM_WriteFloat(camBase + RF1_CAM_Y, camY);
}