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

//4WBHBiCAbp pastebin

#define PI 3.14159265f // 0x40490FDB
#define TAU 6.2831853f // 0x40C90FDB

#define MOHRS_CURRENT_MODE 0x131668 //0x01 = Story Mode, 0x02 = Multiplayer
#define MOHRS_CURRENT_MISSION 0x131669 //Main Game
#define MOHRS_CURRENT_MISSION_LS 0x3c6964 //Level Select

//STORY MODE
#define MOHRS_PLAYERBASE_PTR 0x0035E260
//offsets from camBase
#define MOHRS_FOV 0xE30

#define MOHRS_CAM_X 0x274
#define MOHRS_CAM_Y 0x278

#define MOHRS_TURRET_X 0xE1C
#define MOHRS_TURRET_Y 0xE20
#define MOHRS_TURRET_LIMIT_X 0xE14
#define MOHRS_TURRET_LIMIT_Y 0xE18
#define MOHRS_TURRET_FLAG 0xFC0

//MULTIPLAYER
#define MOHRS_MP_PLAYERBASE_PTR 0x004BB878
//offsets from camBase
/*#define MOHRS_MP_CAM_X 0x274
#define MOHRS_MP_CAM_Y 0x278
#define MOHRS_MP_FOV 0xE30*/



static uint8_t PS2_MOHRS_Status(void);
static void PS2_MOHRS_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Medal of Honor: Rising Sun",
	PS2_MOHRS_Status,
	PS2_MOHRS_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_MOHRISINGSUN = &GAMEDRIVER_INTERFACE;

static uint32_t playerBase = 0;
static uint32_t currentMode = 0;
static uint32_t currentMission = 0;
static uint32_t currentMissionLS = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_MOHRS_Status(void)
{
	// SLUS_207.53
	return (
		PS2_MEM_ReadWord(0x00093390) == 0x534C5553U &&
		PS2_MEM_ReadWord(0x00093394) == 0x5F323037U &&
		PS2_MEM_ReadWord(0x00093398) == 0x2E35333BU
	);
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_MOHRS_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	currentMode = PS2_MEM_ReadUInt8(MOHRS_CURRENT_MODE);

	if(currentMode == 0x01) //Story Mode
	{

		playerBase = PS2_MEM_ReadPointer(MOHRS_PLAYERBASE_PTR);

		currentMission = PS2_MEM_ReadUInt8(MOHRS_CURRENT_MISSION);
		currentMissionLS = PS2_MEM_ReadUInt8(MOHRS_CURRENT_MISSION_LS);

		float looksensitivity = (float)sensitivity / 45.f;
		float scale = 200.f;
		float fov = PS2_MEM_ReadFloat(playerBase + MOHRS_FOV) / 45.f;

		float camX = PS2_MEM_ReadFloat(playerBase + MOHRS_CAM_X);
		float camY = PS2_MEM_ReadFloat(playerBase + MOHRS_CAM_Y);

		float turretX = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_X);
		float turretY = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_Y);

		float turretLimitX = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_LIMIT_X);
		float turretLimitY = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_LIMIT_Y);

		if (PS2_MEM_ReadUInt(playerBase + MOHRS_TURRET_FLAG) == 0x15 && (currentMission == 0x0B || currentMissionLS == 0x0B)) //MISSION 2: Turret Boat Section
		{
			turretX -= (float)xmouse * looksensitivity / scale * fov;
				while (turretX > PI)
					turretX -= TAU;
				while (turretX < -PI)
					turretX += TAU;

			turretY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;
			turretY = ClampFloat(turretY, -1.04719758f, 0.0f);

			PS2_MEM_WriteFloat(playerBase + MOHRS_TURRET_X, (float)turretX);
			PS2_MEM_WriteFloat(playerBase + MOHRS_TURRET_Y, (float)turretY);
		}

		else if (PS2_MEM_ReadUInt(playerBase + MOHRS_TURRET_FLAG) == 0x15)
		{
	
			turretX -= (float)xmouse * looksensitivity / scale * fov;
			turretX = ClampFloat(turretX, -turretLimitX, turretLimitX);

			turretY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;
			turretY = ClampFloat(turretY, -turretLimitY, turretLimitY);

			PS2_MEM_WriteFloat(playerBase + MOHRS_TURRET_X, (float)turretX);
			PS2_MEM_WriteFloat(playerBase + MOHRS_TURRET_Y, (float)turretY);

		}

		else
		{

			camX -= (float)xmouse * looksensitivity / scale * fov;
				while (camX > PI)
					camX -= TAU;
				while (camX < -PI)
					camX += TAU;

			camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;
			camY = ClampFloat(camY, -1.450000048f, 1.450000048f);

			PS2_MEM_WriteFloat(playerBase + MOHRS_CAM_X, (float)camX);
			PS2_MEM_WriteFloat(playerBase + MOHRS_CAM_Y, (float)camY);
		}
	}

	else if (currentMode == 0x02) //Multiplayer 
	{

		playerBase = PS2_MEM_ReadPointer(MOHRS_MP_PLAYERBASE_PTR);

		float looksensitivity = (float)sensitivity / 45.f;
		float scale = 200.f;
		float fov = PS2_MEM_ReadFloat(playerBase + MOHRS_FOV) / 45.f;

		float camX = PS2_MEM_ReadFloat(playerBase + MOHRS_CAM_X);
		float camY = PS2_MEM_ReadFloat(playerBase + MOHRS_CAM_Y);

		float turretX = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_X);
		float turretY = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_Y);

		float turretLimitX = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_LIMIT_X);
		float turretLimitY = PS2_MEM_ReadFloat(playerBase + MOHRS_TURRET_LIMIT_Y);

		if (PS2_MEM_ReadUInt(playerBase + MOHRS_TURRET_FLAG) == 0x15)
		{
	
			turretX -= (float)xmouse * looksensitivity / scale * fov;
			turretX = ClampFloat(turretX, -turretLimitX, turretLimitX);

			turretY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;
			turretY = ClampFloat(turretY, -turretLimitY, turretLimitY);

			PS2_MEM_WriteFloat(playerBase + MOHRS_TURRET_X, (float)turretX);
			PS2_MEM_WriteFloat(playerBase + MOHRS_TURRET_Y, (float)turretY);

		}

		else	
			
		{

			camX -= (float)xmouse * looksensitivity / scale * fov;
				while (camX > PI)
					camX -= TAU;
				while (camX < -PI)
					camX += TAU;

			camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;
			camY = ClampFloat(camY, -1.450000048f, 1.450000048f);

			PS2_MEM_WriteFloat(playerBase + MOHRS_CAM_X, (float)camX);
			PS2_MEM_WriteFloat(playerBase + MOHRS_CAM_Y, (float)camY);
		}
	}

	else
	{
		return;
	}
}