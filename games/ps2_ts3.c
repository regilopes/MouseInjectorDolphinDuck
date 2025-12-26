//==========================================================================
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

#define MAPMAKERXMIN 1.f // mapmaker cursor limits
#define MAPMAKERYMIN 0.f
#define MAPMAKERXMAX 98.f
#define MAPMAKERYMAX 96.f
// TS3 ADDRESSES - OFFSET ADDRESSES BELOW (REQUIRES PLAYERBASE TO USE)
#define TS3_camy 0x108
#define TS3_camx 0x104
#define TS3_fov 0x384 // 0x384
#define TS3_turrety 0x1C
#define TS3_turretx 0x24
#define TS3_crosshairy 0xE14 // 0x7A0
#define TS3_crosshairx 0xE10 // 0x79C
#define TS3_guny 0xE0C // 0x7A8
#define TS3_gunx 0xE08 // 0x7A4
// STATIC ADDRESSES BELOW
#define TS3_playerbase 0x00653AAC // playable character pointer
#define TS3_fovbase 0x00653A48 // fov base pointer
#define TS3_turretbase 0x0066BE20 // turret base pointer
#define TS3_crosshairsetting 0x00629034 // crosshair settings (1 == on and moving)
#define TS3_yaxislimit 0x00653ABC
#define TS3_mapmakery 0x00643694
#define TS3_mapmakerx 0x00643690

static uint8_t TS3_Status(void);
static void TS3_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"TimeSplitters: Future Perfect",
	TS3_Status,
	TS3_Inject,
	1, // 1000 Hz tickrate
	1 // crosshair sway supported for driver
};

const GAMEDRIVER *GAME_PS2_TS3 = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t TS3_Status(void)
{
	return (PS2_MEM_ReadWord(0x008264CC) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x008264D0) == 0x5F323131U &&
			PS2_MEM_ReadWord(0x008264D4) == 0x2E34383BU); // check game header to see if it matches TS3
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void TS3_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	if(PS2_MEM_ReadInt(TS3_yaxislimit) != 0x42A00000) // overwrite y axis limit from 60 degrees (SP) or 75 degrees (MP)
		PS2_MEM_WriteFloat(TS3_yaxislimit, 80.f);
	const float looksensitivity = (float)sensitivity / 40.f;
	const float crosshairsensitivity = ((float)crosshair / 100.f) * looksensitivity;
	float cursorx = PS2_MEM_ReadFloat(TS3_mapmakerx), cursory = PS2_MEM_ReadFloat(TS3_mapmakery);
	if(cursorx >= MAPMAKERXMIN && cursorx <= MAPMAKERXMAX && cursory >= MAPMAKERYMIN && cursory <= MAPMAKERYMAX) // if mapmaker cursor is safe to inject
	{
		cursorx += (float)xmouse / 7.5f * looksensitivity; // calculate mapmaker cursor movement
		cursory += (float)ymouse / 7.5f * looksensitivity;
		PS2_MEM_WriteFloat(TS3_mapmakerx, ClampFloat(cursorx, MAPMAKERXMIN, MAPMAKERXMAX));
		PS2_MEM_WriteFloat(TS3_mapmakery, ClampFloat(cursory, MAPMAKERYMIN, MAPMAKERYMAX));
	}
	const uint32_t playerbase = PS2_MEM_ReadUInt(TS3_playerbase);
	const uint32_t fovbase = PS2_MEM_ReadUInt(TS3_fovbase);
	uint32_t turretbase = PS2_MEM_ReadUInt(TS3_turretbase);
	turretbase = PS2_MEM_ReadUInt(turretbase + 0x67C); // get actual turret pointer

	float turrety = PS2_MEM_ReadFloat(turretbase + TS3_turrety);
	float turretx = PS2_MEM_ReadFloat(turretbase + TS3_turretx);
	if(!playerbase || !fovbase) // if playerbase or fovbase are invalid
	 	return;
	float camx = PS2_MEM_ReadFloat(playerbase + TS3_camx);
	float camy = PS2_MEM_ReadFloat(playerbase + TS3_camy);
	const float fov = PS2_MEM_ReadFloat(fovbase + TS3_fov);
	const float yaxislimit = PS2_MEM_ReadFloat(TS3_yaxislimit);

	if(/*camx >= 0 && camx < 360 &&*/camy >= -yaxislimit && camy <= yaxislimit && fov <= 55 && fov > 3)
	{
		camy += (float)(!invertpitch ? -ymouse : ymouse) / 10.f * looksensitivity * (fov / 55.f); // normal calculation method for Y
		camx -= (float)xmouse / 10.f * looksensitivity * (fov / 55.f); // normal calculation method for X
		/*while(camx < 0)
		camx += 360;
		while(camx >= 360)
		camx -= 360;*/
		camy = ClampFloat(camy, -yaxislimit, yaxislimit);
		PS2_MEM_WriteFloat(playerbase + TS3_camy, camy);
		PS2_MEM_WriteFloat(playerbase + TS3_camx, camx);

		
		turrety -= (float)(!invertpitch ? -ymouse : ymouse) / 10.f * looksensitivity * (fov / 55.f); // normal calculation method for Y
		turretx -= (float)xmouse / 10.f * looksensitivity * (fov / 55.f); // normal calculation method for X
		PS2_MEM_WriteFloat(turretbase + TS3_turrety, turrety);
		PS2_MEM_WriteFloat(turretbase + TS3_turretx, turretx);

		if(crosshair && PS2_MEM_ReadInt(TS3_crosshairsetting) <= 1) // if crosshair sway is enabled and turned on in-game
		{
			float crosshairx = PS2_MEM_ReadFloat(playerbase + TS3_crosshairx), gunx = PS2_MEM_ReadFloat(playerbase + TS3_gunx); // after camera x and y have been calculated and injected, calculate the crosshair/gun sway
			crosshairx += (float)xmouse / 80.f * crosshairsensitivity * (fov / 55.f);
			gunx += (float)xmouse / 80.f * crosshairsensitivity * (fov / 55.f) / 1.5f;
			PS2_MEM_WriteFloat(playerbase + TS3_crosshairx, ClampFloat(crosshairx, -1.f, 1.f));
			PS2_MEM_WriteFloat(playerbase + TS3_gunx, ClampFloat(gunx, -1.f, 1.f));
			float crosshairy = PS2_MEM_ReadFloat(playerbase + TS3_crosshairy), guny = PS2_MEM_ReadFloat(playerbase + TS3_guny);
			crosshairy += (float)(!invertpitch ? ymouse : -ymouse) / 80.f * crosshairsensitivity * (fov / 55.f);
			guny += (float)(!invertpitch ? ymouse : -ymouse) / 80.f * crosshairsensitivity * (fov / 55.f) / 1.5f;
			PS2_MEM_WriteFloat(playerbase + TS3_crosshairy, ClampFloat(crosshairy, -1.f, 1.f));
			PS2_MEM_WriteFloat(playerbase + TS3_guny, ClampFloat(guny, -1.f, 1.f));
		}
	}
	//printf("turretbase: %08X\n", turretbase);
}