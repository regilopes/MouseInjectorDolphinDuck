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

#define PI 3.14159265f // 0x40490FDB
#define TAU 6.2831853f // 0x40C90FDB
#define CROSSHAIRX 0.296f // 0x3E978D50
#define CROSSHAIRY 0.39999999f // 0xBECCCCCC
#define SENTRYMINY -0.1616991162f // 0xBE259474
#define SENTRYMAXY 0.1349733025f // 0x3E0A3671
#define SENTRYFOVBASE 41.25f // 0x42250000
// NF ADDRESSES - OFFSET ADDRESSES BELOW (REQUIRES PLAYERBASE TO USE)
#define PS2_NF_camx 0x54
#define PS2_NF_camy 0x9B8
#define PS2_NF_fov 0x9E4
#define PS2_NF_crosshairx 0x228
#define PS2_NF_crosshairy 0x22C
#define PS2_NF_health 0x9A4
//#define PS2_NF_lookspring 0x80B96FA8 - 0x80B96DEC
#define PS2_NF_sentryx 0x1DC
#define PS2_NF_sentryy 0x1D8
// STATIC ADDRESSES BELOW
#define PS2_NF_playerbase 0x298FD4 // playable character pointer
#define PS2_NF_sentrybase 0x333690 // sentry interface pointer (heli/jet ski)
#define PS2_NF_sentryfov 0x3341C4 // sentry fov
#define PS2_NF_pauseflag 0x2A4980

static uint8_t PS2_NF_Status(void);
static void PS2_NF_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"007: NightFire",
	PS2_NF_Status,
	PS2_NF_Inject,
	1, // 1000 Hz tickrate
	1 // crosshair sway supported for driver
};

const GAMEDRIVER *GAME_PS2_NF = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_NF_Status(void)
{
	//SLUS_205.79 (0x93390) (0x90708)
	// 53 4C 55 53 | 5F 32 30 35 | 2E 37 39 3B  
	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553 &&
			PS2_MEM_ReadWord(0x93394) == 0x5F323035 &&
			PS2_MEM_ReadWord(0x93398) == 0x2E37393B) ||

			(PS2_MEM_ReadWord(0x90708) == 0x534C5553 &&
			PS2_MEM_ReadWord(0x9070C) == 0x5F323035 &&
			PS2_MEM_ReadWord(0x90710) == 0x2E37393B));
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_NF_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	const uint32_t playerbase = PS2_MEM_ReadUInt(PS2_NF_playerbase);
	const float looksensitivity = (float)sensitivity / 40.f;
	const float crosshairsensitivity = ((float)crosshair / 100.f) * looksensitivity;
	if(PS2WITHINMEMRANGE(playerbase)) // if playerbase is valid
	{
		// if(PS2_MEM_ReadInt(playerbase + PS2_NF_lookspring) == 0x03010002) // disable lookspring when spawned
		// 	PS2_MEM_WriteInt(playerbase + PS2_NF_lookspring, 0x01010002);
		float camx = PS2_MEM_ReadFloat(playerbase + PS2_NF_camx);
		float camy = PS2_MEM_ReadFloat(playerbase + PS2_NF_camy);
		const float fov = PS2_MEM_ReadFloat(playerbase + PS2_NF_fov);
		const float hp = PS2_MEM_ReadFloat(playerbase + PS2_NF_health);
		const uint32_t pauseflag = PS2_MEM_ReadUInt(PS2_NF_pauseflag);
		if(camx >= -PI && camx <= PI && camy >= -1.f && camy <= 1.f && fov >= 1.f && hp > 0 && !pauseflag)
		{
			camx -= (float)xmouse / 10.f * looksensitivity / (360.f / TAU) / (fov / 1.f); // normal calculation method for X
			camy += (float)(!invertpitch ? -ymouse : ymouse) / 10.f * looksensitivity / 90.f / (fov / 1.f); // normal calculation method for Y
			while(camx <= -PI)
				camx += TAU;
			while(camx >= PI)
				camx -= TAU;
			camy = ClampFloat(camy, -1.f, 1.f);
			PS2_MEM_WriteFloat(playerbase + PS2_NF_camx, camx);
			PS2_MEM_WriteFloat(playerbase + PS2_NF_camy, camy);
			if(crosshair) // if crosshair sway is enabled
			{
				float crosshairx = PS2_MEM_ReadFloat(playerbase + PS2_NF_crosshairx); // after camera x and y have been calculated and injected, calculate the crosshair/gun sway
				float crosshairy = PS2_MEM_ReadFloat(playerbase + PS2_NF_crosshairy);
				crosshairx += (float)xmouse / 80.f * crosshairsensitivity / (fov / 1.f);
				crosshairy += (float)(!invertpitch ? -ymouse : ymouse) / 80.f * crosshairsensitivity / (fov / 1.f);
				PS2_MEM_WriteFloat(playerbase + PS2_NF_crosshairx, ClampFloat(crosshairx, -CROSSHAIRX, CROSSHAIRX));
				PS2_MEM_WriteFloat(playerbase + PS2_NF_crosshairy, ClampFloat(crosshairy, -CROSSHAIRY, CROSSHAIRY));
			}
		}
	}
	else // if playerbase is invalid, check for sentry mode
	{
		const uint32_t sentrybase = PS2_MEM_ReadUInt(PS2_NF_sentrybase);
		if(PS2NOTWITHINMEMRANGE(sentrybase)) // if sentrybase is invalid
			return;
		float sentryx = PS2_MEM_ReadFloat(sentrybase + PS2_NF_sentryx);
		float sentryy = PS2_MEM_ReadFloat(sentrybase + PS2_NF_sentryy);
		const float fov = PS2_MEM_ReadFloat(PS2_NF_sentryfov);
		if(sentryx >= -1.f && sentryx <= 1.f)
		{
			sentryx += (float)xmouse / 10.f * looksensitivity / 360.f / (SENTRYFOVBASE / fov);
			sentryy += (float)(!invertpitch ? ymouse : -ymouse) / 10.f * looksensitivity / (90.f / (SENTRYMAXY - SENTRYMINY)) / (SENTRYFOVBASE / fov);
			while(sentryx <= -1.f)
				sentryx += 1.f;
			while(sentryx >= 1.f)
				sentryx -= 1.f;
			sentryy = ClampFloat(sentryy, SENTRYMINY, SENTRYMAXY);
			PS2_MEM_WriteFloat(sentrybase + PS2_NF_sentryx, sentryx);
			PS2_MEM_WriteFloat(sentrybase + PS2_NF_sentryy, sentryy);
		}
	}
}